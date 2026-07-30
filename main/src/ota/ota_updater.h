#ifndef OTA_UPDATER_H
#define OTA_UPDATER_H

#include <Arduino.h>

extern const unsigned long OTA_CHECK_INTERVAL;
extern unsigned long lastOtaCheckTime;

void checkForUpdates();

#endif // OTA_UPDATER_H
