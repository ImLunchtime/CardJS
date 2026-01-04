#include <Arduino.h>
#include "M5Cardputer.h"
#include "M5GFX.h"
#include <SPI.h>
#include <SD.h>
#include <WiFi.h>
#include <HTTPClient.h>

#include "config.h"
#include "globals.h"
#include "fs_utils.h"
#include "js_bindings.h"
#include "duktape_port.h"

void setup() {
  Serial.begin(9600);

  SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);

  auto cfg = M5.config();
  M5Cardputer.begin(cfg, true);

  loadConfigFile(ssid,password);
  WiFi.begin(ssid, password);

  script = readBootFile();
}

void loop() {
  M5Cardputer.Display.fillRect(0,0,M5Cardputer.Display.width(),M5Cardputer.Display.height(),BLACK);
  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setTextSize(2);
  M5Cardputer.Display.setTextColor(WHITE);

  duk_context *ctx = duk_create_heap_default();

  registerDukBindings(ctx);

  duk_push_string(ctx, script.c_str());
  if (duk_peval(ctx) != 0) {
      printf("eval failed: %s\n", duk_safe_to_string(ctx, -1));
  } else {
      printf("result is: %s\n", duk_safe_to_string(ctx, -1));
  }
  duk_pop(ctx);

  duk_destroy_heap(ctx);

  delay(1000);
}

