#include "globals.h"

String ssid = "";
String password = "";
String headers[20];
String script = "drawString('Something wrong.', 4, 4);";
String pendingScriptPath = "";
bool pendingScriptIsSpiffs = true;
bool pendingScriptChange = false;
String currentScriptPath = "/boot.js";

HTTPClient http;
