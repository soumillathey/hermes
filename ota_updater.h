#ifndef OTA_UPDATER_H
#define OTA_UPDATER_H

#include <Arduino.h>

const unsigned long OTA_CHECK_INTERVAL = 3600000; // Check every 1 hour (in ms)
extern unsigned long lastOtaCheckTime;

void checkForUpdates();

#endif // OTA_UPDATER_H
