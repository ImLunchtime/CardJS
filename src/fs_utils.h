#pragma once

#include <Arduino.h>
#include <FS.h>

String readBootFile();
void loadConfigFile(String &ssid, String &password);
void writeFile(fs::FS &fs, const char *path, const char *message);
void appendFile(fs::FS &fs, const char *path, const char *message);
void renameFile(fs::FS &fs, const char *path1, const char *path2);
void deleteFile(fs::FS &fs, const char *path);

