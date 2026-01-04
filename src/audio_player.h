#pragma once

#include <Arduino.h>

void audioPlayerInit();
void audioPlayerShutdown();
void audioPlayerPlaySD(const char *path, int playMode);
void audioPlayerStop();
void audioPlayerSetVolume(int volumePercent);
bool audioPlayerIsPlaying();
