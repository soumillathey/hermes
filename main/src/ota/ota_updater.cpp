#include "ota_updater.h"
#include "../led/seven_segment.h"
#include "../led/rgb_led.h"
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

static const String CURRENT_VERSION = "1.0.2";
static const String OTA_VERSION_URL = "https://raw.githubusercontent.com/SoumilLathey/gluvok-hardware-ota/main/firmware/version.json";

const unsigned long OTA_CHECK_INTERVAL = 3600000;
unsigned long lastOtaCheckTime = 0;

void checkForUpdates() {
  if (WiFi.status() != WL_CONNECTED) return;

  Serial.println("[OTA] Checking for firmware updates...");

  WiFiClientSecure secureClient;
  secureClient.setInsecure();

  HTTPClient http;
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  if (http.begin(secureClient, OTA_VERSION_URL)) {
    http.setTimeout(10000);
    http.addHeader("User-Agent", "ESP32");
    http.addHeader("Connection", "close");

    int httpResponseCode = http.GET();

    if (httpResponseCode == 200) {
      String payload = http.getString();
      int versionIdx = payload.indexOf("\"version\"");
      int urlIdx = payload.indexOf("\"url\"");

      if (versionIdx != -1 && urlIdx != -1) {
        int verStart = payload.indexOf("\"", payload.indexOf(":", versionIdx) + 1) + 1;
        int verEnd = payload.indexOf("\"", verStart);
        String remoteVersion = payload.substring(verStart, verEnd);
        remoteVersion.trim();

        int urlStart = payload.indexOf("\"", payload.indexOf(":", urlIdx) + 1) + 1;
        int urlEnd = payload.indexOf("\"", urlStart);
        String binUrl = payload.substring(urlStart, urlEnd);
        binUrl.trim();

        // Close the version check HTTP connection BEFORE initiating the firmware update
        http.end();

        if (remoteVersion != CURRENT_VERSION && binUrl.length() > 0) {
          Serial.printf("[OTA] New firmware found (%s -> %s). Starting download...\n", CURRENT_VERSION.c_str(), remoteVersion.c_str());
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
              Serial.printf("[OTA] Update failed (Error %d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
              displayErrorCode(SEG_CODE_OTA_ERR);
              delay(3000);
              updateRGBStatus(); // Revert 7-segment back to '0' (Ready) and RGB to Blue
              break;
            case HTTP_UPDATE_NO_UPDATES:
              Serial.println("[OTA] No updates available.");
              updateRGBStatus(); // Revert 7-segment back to '0' (Ready) and RGB to Blue
              break;
            case HTTP_UPDATE_OK:
              Serial.println("[OTA] Firmware updated successfully! Rebooting...");
              displayErrorCode(SEG_CODE_READY);
              break;
          }
          return;
        } else {
          Serial.printf("[OTA] Firmware is up to date (v%s).\n", CURRENT_VERSION.c_str());
          updateRGBStatus();
        }
      } else {
        http.end();
      }
    } else {
      Serial.printf("[OTA] Version check failed, HTTP error: %d (%s)\n", httpResponseCode, http.errorToString(httpResponseCode).c_str());
      http.end();
    }
  }
}
