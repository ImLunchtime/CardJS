#include <Arduino.h>
#include "M5Cardputer.h"
#include "M5GFX.h"
#include <SPI.h>
#include <SD.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPIFFS.h>

#include "config.h"
#include "globals.h"
#include "fs_utils.h"
#include "js_bindings.h"
#include "duktape_port.h"

static duk_context *ctx = nullptr;

static void callJsFunction(const char *name) {
  if (!ctx) {
    return;
  }
  duk_get_global_string(ctx, name);
  if (!duk_is_function(ctx, -1)) {
    duk_pop(ctx);
    return;
  }
  if (duk_pcall(ctx, 0) != 0) {
    printf("%s error: %s\n", name, duk_safe_to_string(ctx, -1));
  }
  duk_pop(ctx);
}

static void reloadScriptFromPending() {
  if (!pendingScriptChange) {
    return;
  }
  pendingScriptChange = false;

  if (ctx) {
    duk_destroy_heap(ctx);
    ctx = nullptr;
  }

  ctx = duk_create_heap_default();
  registerDukBindings(ctx);

  M5Cardputer.Display.fillRect(0,0,M5Cardputer.Display.width(),M5Cardputer.Display.height(),BLACK);
  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setTextSize(2);
  M5Cardputer.Display.setTextColor(WHITE);

  String s;
  if (pendingScriptIsSpiffs) {
    if (!SPIFFS.begin(true)) {
      s = "drawString('Failed to mount SPIFFS.', 4, 4);";
    } else {
      File file = SPIFFS.open(pendingScriptPath.c_str(), "r");
      if (!file) {
        s = "drawString('Script not found.', 4, 4);";
      } else {
        while (file.available()) {
          s += (char)file.read();
        }
        file.close();
      }
    }
  } else {
    SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);
    if (!SD.begin(SD_SPI_CS_PIN, SPI, 25000000)) {
      s = "drawString('Failed to mount SD.', 4, 4);";
    } else {
      File file = SD.open(pendingScriptPath.c_str(), "r");
      if (!file) {
        s = "drawString('Script not found.', 4, 4);";
      } else {
        while (file.available()) {
          s += (char)file.read();
        }
        file.close();
      }
    }
  }

  if (s.length() == 0) {
    s = "drawString('Empty script.', 4, 4);";
  }
  script = s;
  currentScriptPath = pendingScriptPath;

  duk_push_string(ctx, script.c_str());
  if (duk_peval(ctx) != 0) {
      printf("switch eval failed: %s\n", duk_safe_to_string(ctx, -1));
  } else {
      printf("switch result: %s\n", duk_safe_to_string(ctx, -1));
  }
  duk_pop(ctx);

  callJsFunction("setup");
}

void setup() {
  Serial.begin(9600);

  SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);

  auto cfg = M5.config();
  M5Cardputer.begin(cfg, true);

  pinMode(GPIO_NUM_5, OUTPUT);
  digitalWrite(GPIO_NUM_5, HIGH);

  loadConfigFile(ssid,password);
  WiFi.begin(ssid, password);

  ctx = duk_create_heap_default();
  registerDukBindings(ctx);

  M5Cardputer.Display.fillRect(0,0,M5Cardputer.Display.width(),M5Cardputer.Display.height(),BLACK);
  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setTextSize(2);
  M5Cardputer.Display.setTextColor(WHITE);

  script = readBootFile();
  currentScriptPath = "/boot.js";

  duk_push_string(ctx, script.c_str());
  if (duk_peval(ctx) != 0) {
      printf("boot eval failed: %s\n", duk_safe_to_string(ctx, -1));
  } else {
      printf("boot result: %s\n", duk_safe_to_string(ctx, -1));
  }
  duk_pop(ctx);

  callJsFunction("setup");
}

void loop() {
  M5Cardputer.update();
  if (currentScriptPath != "/boot.js" && M5Cardputer.BtnA.wasPressed()) {
    pendingScriptPath = "/boot.js";
    pendingScriptIsSpiffs = true;
    pendingScriptChange = true;
  }
  reloadScriptFromPending();
  callJsFunction("loop");
  delay(10);
}
