#pragma once

#include <Arduino.h>
#include <HTTPClient.h>

extern String ssid;
extern String password;
extern String headers[20];
extern String script;
extern String pendingScriptPath;
extern bool pendingScriptIsSpiffs;
extern bool pendingScriptChange;
extern String currentScriptPath;

extern HTTPClient http;
