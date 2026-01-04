#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>

#include "fs_utils.h"

String readBootFile() {
    String mountError = "drawString('Can't mount SPIFFS.', 4, 4);";
    String fileError = "drawString('No boot.js file.', 4, 4);";
    
    if (!SPIFFS.begin(true)) {
        Serial.println("Failed to mount SPIFFS");
        return mountError;
    }

    File file = SPIFFS.open("/boot.js", "r");
    if (!file) {
        Serial.println("No boot.js file");
        return fileError;
    }

    String s;
    Serial.println("Read from file");
    while (file.available()) {
        s += (char)file.read();
    }
    file.close();
    Serial.println("loaded file:");
    Serial.println(s);
    return s;
}

void loadConfigFile(String &ssid, String &password) {
    String mountError = "Can't mount SPIFFS.";
    String fileError = "No config.txt file.";
    String formatError = "Invalid config file format.";

    if (!SPIFFS.begin(true)) {
        Serial.println(mountError);
        return;
    }

    File file = SPIFFS.open("/config.txt", "r");
    if (!file) {
        Serial.println(fileError);
        return;
    }

    String line;
    bool ssidFound = false;
    bool passwordFound = false;
    while (file.available()) {
        line = file.readStringUntil('\n');
        line.trim();

        if (line.startsWith("SSID:")) {
            String extractedSSID = line.substring(5);
            extractedSSID.trim();
            ssid = extractedSSID;
            ssidFound = true;
        } else if (line.startsWith("PASSWORD:")) {
            String extractedPassword = line.substring(9);
            extractedPassword.trim();
            password = extractedPassword;
            passwordFound = true;
        }
    }
    file.close();

    if (!ssidFound || !passwordFound) {
        Serial.println(formatError);
        return;
    }

    Serial.println("Config loaded:");
    Serial.print("SSID: ");
    Serial.println(ssid);
    Serial.print("Passkey: ");
    Serial.println(password);
}

void writeFile(fs::FS &fs, const char *path, const char *message) {
    Serial.print("Writing file");

    File file = fs.open(path, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }
    if (file.print(message)) {
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void appendFile(fs::FS &fs, const char *path, const char *message) {
    Serial.print("Appending to file");

    File file = fs.open(path, FILE_APPEND);
    if (!file) {
        Serial.println("Failed to open file for appending");
        return;
    }
    if (file.print(message)) {
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}

void renameFile(fs::FS &fs, const char *path1, const char *path2) {
    Serial.print("Renaming file");
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

void deleteFile(fs::FS &fs, const char *path) {
    Serial.print("Deleting file");
    if (fs.remove(path)) {
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}
