#include "audio_player.h"

#include "M5Cardputer.h"
#include <SPI.h>
#include <SD.h>

#include <AudioOutput.h>
#include <AudioFileSourceSD.h>
#include <AudioFileSourceID3.h>
#include <AudioGeneratorMP3.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>

#include "config.h"

class AudioOutputM5Speaker : public AudioOutput {
public:
  AudioOutputM5Speaker(m5::Speaker_Class *speaker, uint8_t virtualChannel = 0)
      : speaker(speaker), virtualChannel(virtualChannel) {}

  bool begin() override { return true; }

  bool ConsumeSample(int16_t sample[2]) override {
    if (bufferIndex < bufferSize) {
      buffer[bufferSlot][bufferIndex] = sample[0];
      buffer[bufferSlot][bufferIndex + 1] = sample[1];
      bufferIndex += 2;
      return true;
    }
    flush();
    return false;
  }

  void flush() override {
    if (bufferIndex) {
      speaker->playRaw(buffer[bufferSlot], bufferIndex, hertz, true, 1,
                       virtualChannel);
      bufferSlot = bufferSlot < 2 ? bufferSlot + 1 : 0;
      bufferIndex = 0;
    }
  }

  bool stop() override {
    flush();
    speaker->stop(virtualChannel);
    return true;
  }

private:
  m5::Speaker_Class *speaker;
  uint8_t virtualChannel;
  static constexpr size_t bufferSize = 640;
  int16_t buffer[3][bufferSize];
  size_t bufferIndex = 0;
  size_t bufferSlot = 0;
};

enum AudioCommand {
  AUDIO_CMD_PLAY,
  AUDIO_CMD_STOP,
  AUDIO_CMD_SET_VOLUME,
  AUDIO_CMD_SHUTDOWN
};

struct AudioTaskCommand {
  AudioCommand cmd;
  int param;
  int playMode;
  char filePath[256];
};

static TaskHandle_t audioTaskHandle = nullptr;
static QueueHandle_t audioCommandQueue = nullptr;
static SemaphoreHandle_t audioStateMutex = nullptr;

static AudioFileSourceSD *audioFile = nullptr;
static AudioFileSourceID3 *id3Source = nullptr;
static AudioOutputM5Speaker *audioOutput = nullptr;
static AudioGeneratorMP3 *mp3 = nullptr;

static bool audioInitialized = false;
static bool audioPlaying = false;
static bool audioLoop = false;
static int audioVolume = 50;
static String audioCurrentPath;

static void audioPlayerStopInternal();
static void audioPlayerStartInternal(const char *path, int playMode);

static void audioTaskFunction(void *parameter) {
  audioFile = new AudioFileSourceSD();
  if (!audioFile) {
    vTaskDelete(nullptr);
    return;
  }

  audioOutput = new AudioOutputM5Speaker(&M5Cardputer.Speaker, 0);
  if (!audioOutput) {
    delete audioFile;
    audioFile = nullptr;
    vTaskDelete(nullptr);
    return;
  }

  mp3 = new AudioGeneratorMP3();
  if (!mp3) {
    delete audioOutput;
    audioOutput = nullptr;
    delete audioFile;
    audioFile = nullptr;
    vTaskDelete(nullptr);
    return;
  }

  id3Source = nullptr;

  if (!audioOutput->begin()) {
    delete mp3;
    mp3 = nullptr;
    delete audioOutput;
    audioOutput = nullptr;
    delete audioFile;
    audioFile = nullptr;
    vTaskDelete(nullptr);
    return;
  }

  audioOutput->SetRate(44100);
  audioOutput->SetBitsPerSample(16);
  audioOutput->SetChannels(2);

  M5Cardputer.Speaker.setVolume((audioVolume * 255) / 100);

  AudioTaskCommand command;

  for (;;) {
    if (mp3 && mp3->isRunning()) {
      if (!mp3->loop()) {
        if (audioLoop && audioCurrentPath.length() > 0) {
          audioPlayerStartInternal(audioCurrentPath.c_str(), audioLoop ? 1 : 0);
        } else {
          audioPlayerStopInternal();
        }
      }
    }

    if (xQueueReceive(audioCommandQueue, &command, pdMS_TO_TICKS(1)) ==
        pdTRUE) {
      switch (command.cmd) {
      case AUDIO_CMD_PLAY:
        audioPlayerStartInternal(command.filePath, command.playMode);
        break;
      case AUDIO_CMD_STOP:
        audioPlayerStopInternal();
        break;
      case AUDIO_CMD_SET_VOLUME:
        audioVolume = command.param;
        if (audioVolume < 0) {
          audioVolume = 0;
        }
        if (audioVolume > 100) {
          audioVolume = 100;
        }
        M5Cardputer.Speaker.setVolume((audioVolume * 255) / 100);
        break;
      case AUDIO_CMD_SHUTDOWN:
        audioPlayerStopInternal();
        vTaskDelete(nullptr);
        return;
      }
    }

    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

static void audioEnsureInitialized() {
  if (audioInitialized) {
    return;
  }

  audioCommandQueue = xQueueCreate(8, sizeof(AudioTaskCommand));
  if (!audioCommandQueue) {
    return;
  }

  audioStateMutex = xSemaphoreCreateMutex();

  BaseType_t result = xTaskCreatePinnedToCore(
      audioTaskFunction, "AudioPlayerTask", 8192, nullptr, 1, &audioTaskHandle,
      1);
  if (result != pdPASS) {
    vQueueDelete(audioCommandQueue);
    audioCommandQueue = nullptr;
    if (audioStateMutex) {
      vSemaphoreDelete(audioStateMutex);
      audioStateMutex = nullptr;
    }
    return;
  }

  audioInitialized = true;
}

static void audioPlayerStartInternal(const char *path, int playMode) {
  if (!path) {
    return;
  }

  audioPlayerStopInternal();

  SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);
  if (!SD.begin(SD_SPI_CS_PIN, SPI, 25000000)) {
    return;
  }

  if (!audioFile) {
    return;
  }
  if (!audioFile->open(path)) {
    return;
  }

  if (id3Source) {
    delete id3Source;
    id3Source = nullptr;
  }
  id3Source = new AudioFileSourceID3(audioFile);
  if (!id3Source) {
    audioFile->close();
    return;
  }

  if (!mp3) {
    return;
  }

  if (!mp3->begin(id3Source, audioOutput)) {
    delete id3Source;
    id3Source = nullptr;
    audioFile->close();
    return;
  }

  audioCurrentPath = String(path);
  audioLoop = playMode == 1;
  audioPlaying = true;
}

static void audioPlayerStopInternal() {
  if (mp3 && mp3->isRunning()) {
    mp3->stop();
  }

  if (audioOutput) {
    audioOutput->flush();
    audioOutput->stop();
  }

  if (id3Source) {
    delete id3Source;
    id3Source = nullptr;
  }

  if (audioFile && audioFile->isOpen()) {
    audioFile->close();
  }

  audioPlaying = false;
}

void audioPlayerInit() { audioEnsureInitialized(); }

void audioPlayerShutdown() {
  if (!audioInitialized) {
    return;
  }
  if (!audioCommandQueue) {
    return;
  }
  AudioTaskCommand command;
  command.cmd = AUDIO_CMD_SHUTDOWN;
  command.param = 0;
  command.playMode = 0;
  command.filePath[0] = 0;
  xQueueSend(audioCommandQueue, &command, portMAX_DELAY);
}

void audioPlayerPlaySD(const char *path, int playMode) {
  audioEnsureInitialized();
  if (!audioCommandQueue || !path) {
    return;
  }
  AudioTaskCommand command;
  command.cmd = AUDIO_CMD_PLAY;
  command.param = 0;
  command.playMode = playMode;
  strncpy(command.filePath, path, sizeof(command.filePath) - 1);
  command.filePath[sizeof(command.filePath) - 1] = 0;
  xQueueSend(audioCommandQueue, &command, portMAX_DELAY);
}

void audioPlayerStop() {
  if (!audioInitialized || !audioCommandQueue) {
    return;
  }
  AudioTaskCommand command;
  command.cmd = AUDIO_CMD_STOP;
  command.param = 0;
  command.playMode = 0;
  command.filePath[0] = 0;
  xQueueSend(audioCommandQueue, &command, portMAX_DELAY);
}

void audioPlayerSetVolume(int volumePercent) {
  audioEnsureInitialized();
  if (!audioCommandQueue) {
    return;
  }
  AudioTaskCommand command;
  command.cmd = AUDIO_CMD_SET_VOLUME;
  command.param = volumePercent;
  command.playMode = 0;
  command.filePath[0] = 0;
  xQueueSend(audioCommandQueue, &command, portMAX_DELAY);
}

bool audioPlayerIsPlaying() {
  if (!audioInitialized) {
    return false;
  }
  bool value = false;
  if (audioStateMutex &&
      xSemaphoreTake(audioStateMutex, pdMS_TO_TICKS(10)) == pdTRUE) {
    value = audioPlaying;
    xSemaphoreGive(audioStateMutex);
  } else {
    value = audioPlaying;
  }
  return value;
}

