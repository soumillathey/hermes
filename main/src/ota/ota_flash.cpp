/**
 * ota_flash.cpp
 * Downloads and flashes the firmware binary from a given URL.
 * Manages 7-segment and RGB status indicators throughout the update.
 */

#include "ota_flash.h"
#include "ota_updater.h"
#include "../led/seven_segment.h"
#include "../led/rgb_led.h"
#include <HTTPUpdate.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

void performOtaUpdate(const String& binUrl, const String& newVersion) {
  displayErrorCode(SEG_CODE_OTA_PROGRESS);
  WiFi.setSleep(false); // Disable modem sleep to prevent TCP stream timeouts

  WiFiClientSecure otaClient;
  otaClient.setInsecure();
  otaClient.setTimeout(30000); // 30s socket timeout for large binary stream

  httpUpdate.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  httpUpdate.rebootOnUpdate(true);

  t_httpUpdate_return ret = httpUpdate.update(otaClient, binUrl);
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("[OTA] Update failed (Error %d): %s\n",
                    httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      displayErrorCode(SEG_CODE_OTA_ERR);
      delay(3000);
      updateRGBStatus(); // Revert display to Ready state
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("[OTA] No updates available.");
      updateRGBStatus();
      break;

    case HTTP_UPDATE_OK:
      Serial.println("[OTA] Firmware updated successfully! Rebooting...");
      displayErrorCode(SEG_CODE_READY);
      break;
  }
}
