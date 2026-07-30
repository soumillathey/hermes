/**
 * ota_flash.h
 * Internal header for the OTA firmware flash executor.
 * Declared here so ota_version.cpp can call into ota_flash.cpp.
 */

#ifndef OTA_FLASH_H
#define OTA_FLASH_H

#include <Arduino.h>

void performOtaUpdate(const String& binUrl, const String& newVersion);

#endif // OTA_FLASH_H
