#include <Arduino.h>
#include "M5Cardputer.h"
#include "M5GFX.h"
#include <SPI.h>
#include <SD.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <string.h>
#include <chrono>

#include "globals.h"
#include "duktape_port.h"
#include "config.h"

static duk_ret_t native_load(duk_context *ctx) {
  script = duk_to_string(ctx, 0);
  return 0;
}

static duk_ret_t native_print(duk_context *ctx) {
  Serial.print(duk_to_string(ctx, 0));
  return 0;
}

static duk_ret_t native_now(duk_context *ctx) {
    using namespace std::chrono;
    auto now = high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    auto millis = duration_cast<milliseconds>(duration).count();
    duk_push_number(ctx, static_cast<double>(millis));
    return 1;
}

static duk_ret_t native_delay(duk_context *ctx) {
  delay(duk_to_number(ctx, 0));
  return 0;
}

static duk_ret_t native_digitalWrite(duk_context *ctx) {
  digitalWrite(duk_to_number(ctx, 0),duk_to_boolean(ctx, 1));
  return 0;
}

static duk_ret_t native_pinMode(duk_context *ctx) {
  pinMode(duk_to_number(ctx, 0),duk_to_number(ctx, 1));
  return 0;
}

static duk_ret_t native_get(duk_context *ctx) {
  duk_idx_t obj_idx;
  if(WiFi.status()== WL_CONNECTED){
      http.begin(duk_to_string(ctx, 0));

      if (duk_is_array(ctx, 1)) {
        duk_uint_t len = duk_get_length(ctx, 1);
        for (duk_uint_t i = 0; i < len; i++) {
            duk_get_prop_index(ctx, 1, i);
            
            if (!duk_is_string(ctx, -1)) {
                duk_pop(ctx);
                duk_error(ctx, DUK_ERR_TYPE_ERROR, "Header array elements must be strings.");
            }

            const char *headerKey = duk_get_string(ctx, -1);
            duk_pop(ctx);
            i++;
            duk_get_prop_index(ctx, 1, i);
            
            if (!duk_is_string(ctx, -1)) {
                duk_pop(ctx);
                duk_error(ctx, DUK_ERR_TYPE_ERROR, "Header array elements must be strings.");
            }

            const char *headerValue = duk_get_string(ctx, -1);
            duk_pop(ctx);
            http.addHeader(headerKey, headerValue);
        }
      }
      
      int httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
        String payload = http.getString();

        obj_idx = duk_push_object(ctx);
        duk_push_int(ctx, httpResponseCode);
        duk_put_prop_string(ctx, obj_idx, "response");
        duk_push_string(ctx, payload.c_str());
        duk_put_prop_string(ctx, obj_idx, "body");

      }
      else {
        String errorMessage = "Error Response";
        obj_idx = duk_push_object(ctx);
        duk_push_int(ctx, 0);
        duk_put_prop_string(ctx, obj_idx, "response");
        duk_push_string(ctx, errorMessage.c_str());
        duk_put_prop_string(ctx, obj_idx, "body");
      }
      http.end();
    }
    else {
      String noWifiMessage = "WIFI Not Connected";
      obj_idx = duk_push_object(ctx);
      duk_push_int(ctx, 0);
      duk_put_prop_string(ctx, obj_idx, "response");
      duk_push_string(ctx, noWifiMessage.c_str());
      duk_put_prop_string(ctx, obj_idx, "body");
    }
  return 1;
}

static duk_ret_t native_color(duk_context *ctx) {
  int color = M5Cardputer.Display.color565(duk_to_int(ctx, 0),duk_to_int(ctx, 1),duk_to_int(ctx, 2));
  duk_push_int(ctx, color);
  return 1;
}

static duk_ret_t native_setTextColor(duk_context *ctx) {
  M5Cardputer.Display.setTextColor(duk_to_int(ctx, 0));
  return 0;
}

static duk_ret_t native_setTextSize(duk_context *ctx) {
  M5Cardputer.Display.setTextSize(duk_to_number(ctx, 0));
  return 0;
}

static duk_ret_t native_drawRect(duk_context *ctx) {
  M5Cardputer.Display.drawRect(duk_to_int(ctx, 0),duk_to_int(ctx, 1),duk_to_int(ctx, 2),duk_to_int(ctx, 3),duk_to_int(ctx, 4));
  return 0;
}

static duk_ret_t native_drawFillRect(duk_context *ctx) {
  M5Cardputer.Display.fillRect(duk_to_int(ctx, 0),duk_to_int(ctx, 1),duk_to_int(ctx, 2),duk_to_int(ctx, 3),duk_to_int(ctx, 4));
  return 0;
}

static duk_ret_t native_drawString(duk_context *ctx) {
  M5Cardputer.Display.drawString(duk_to_string(ctx, 0),duk_to_int(ctx, 1),duk_to_int(ctx, 2));
  return 0;
}

static duk_ret_t native_width(duk_context *ctx) {
  int width = M5Cardputer.Display.width();
  duk_push_int(ctx, width);
  return 1;
}

static duk_ret_t native_height(duk_context *ctx) {
  int height = M5Cardputer.Display.height();
  duk_push_int(ctx, height);
  return 1;
}

static duk_ret_t native_getKeysPressed(duk_context *ctx) {
  duk_push_array(ctx);

  M5Cardputer.update();
    if (M5Cardputer.Keyboard.isChange()) {
        if (M5Cardputer.Keyboard.isPressed()) {
            Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
            int arrayIndex = 0;
            for (auto i : status.word) {
                char str[2] = { i, '\0' };
                duk_push_string(ctx, str);
                duk_put_prop_index(ctx, -2, arrayIndex);
                arrayIndex++;
            }

            if (status.del) {
                duk_push_string(ctx, "Delete");
                duk_put_prop_index(ctx, -2, arrayIndex);
                arrayIndex++;
            }

            if (status.enter) {
                duk_push_string(ctx, "Enter");
                duk_put_prop_index(ctx, -2, arrayIndex);
                arrayIndex++;
            }

            if (status.alt) {
                duk_push_string(ctx, "Alt");
                duk_put_prop_index(ctx, -2, arrayIndex);
                arrayIndex++;
            }

            if (status.tab) {
                duk_push_string(ctx, "Tab");
                duk_put_prop_index(ctx, -2, arrayIndex);
                arrayIndex++;
            }

            if (status.fn) {
                duk_push_string(ctx, "Function");
                duk_put_prop_index(ctx, -2, arrayIndex);
                arrayIndex++;
            }

            if (status.opt) {
                duk_push_string(ctx, "Option");
                duk_put_prop_index(ctx, -2, arrayIndex);
                arrayIndex++;
            }
        }
    }
  return 1;
}

static duk_ret_t native_changeScriptSpiffs(duk_context *ctx) {
  const char *path = duk_to_string(ctx, 0);
  if (!SPIFFS.begin(true)) {
    duk_error(ctx, DUK_ERR_ERROR, "Failed to mount SPIFFS");
  }
  File file = SPIFFS.open(path, "r");
  if (!file) {
    duk_error(ctx, DUK_ERR_ERROR, "SPIFFS script not found");
  }
  file.close();
  pendingScriptPath = String(path);
  pendingScriptIsSpiffs = true;
  pendingScriptChange = true;
  return 0;
}

static duk_ret_t native_changeScriptSD(duk_context *ctx) {
  const char *path = duk_to_string(ctx, 0);
  SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);
  if (!SD.begin(SD_SPI_CS_PIN, SPI, 25000000)) {
    duk_error(ctx, DUK_ERR_ERROR, "Failed to mount SD");
  }
  File file = SD.open(path, "r");
  if (!file) {
    duk_error(ctx, DUK_ERR_ERROR, "SD script not found");
  }
  file.close();
  pendingScriptPath = String(path);
  pendingScriptIsSpiffs = false;
  pendingScriptChange = true;
  return 0;
}

void registerDukBindings(duk_context *ctx) {
  duk_push_c_function(ctx, native_load, 1);
  duk_put_global_string(ctx, "load");
  duk_push_c_function(ctx, native_print, 1);
  duk_put_global_string(ctx, "print");
  duk_push_c_function(ctx, native_now, 0);
  duk_put_global_string(ctx, "now");
  duk_push_c_function(ctx, native_delay, 1);
  duk_put_global_string(ctx, "delay");
  duk_push_c_function(ctx, native_digitalWrite, 2);
  duk_put_global_string(ctx, "digitalWrite");
  duk_push_c_function(ctx, native_pinMode, 2);
  duk_put_global_string(ctx, "pinMode");

  duk_push_c_function(ctx, native_get, 2);
  duk_put_global_string(ctx, "httpGet");

  duk_push_c_function(ctx, native_color, 3);
  duk_put_global_string(ctx, "color");
  duk_push_c_function(ctx, native_setTextColor, 1);
  duk_put_global_string(ctx, "setTextColor");
  duk_push_c_function(ctx, native_setTextSize, 1);
  duk_put_global_string(ctx, "setTextSize");
  duk_push_c_function(ctx, native_drawRect, 5);
  duk_put_global_string(ctx, "drawRect");
  duk_push_c_function(ctx, native_drawFillRect, 5);
  duk_put_global_string(ctx, "drawFillRect");
  duk_push_c_function(ctx, native_drawString, 3);
  duk_put_global_string(ctx, "drawString");
  duk_push_c_function(ctx, native_width, 0);
  duk_put_global_string(ctx, "width");
  duk_push_c_function(ctx, native_height, 0);
  duk_put_global_string(ctx, "height");

  duk_push_c_function(ctx, native_getKeysPressed, 0);
  duk_put_global_string(ctx, "getKeysPressed");

  duk_push_c_function(ctx, native_changeScriptSpiffs, 1);
  duk_put_global_string(ctx, "changeScriptSpiffs");
  duk_push_c_function(ctx, native_changeScriptSD, 1);
  duk_put_global_string(ctx, "changeScriptSD");
}
