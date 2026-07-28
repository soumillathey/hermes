#include "ota_updater.h"
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

static const String CURRENT_VERSION = "1.0.2";
static const String OTA_VERSION_URL = "https://raw.githubusercontent.com/SoumilLathey/gluvok-hardware-ota/main/firmware/version.json";

unsigned long lastOtaCheckTime = 0;

void checkForUpdates() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[OTA] Skipped: No Wi-Fi network connection.");
    return;
  }

  Serial.println("[OTA] Checking for updates...");

  WiFiClientSecure secureClient;
  secureClient.setInsecure();

  HTTPClient http;
  if (http.begin(secureClient, OTA_VERSION_URL)) {
    http.setTimeout(10000);
    int httpResponseCode = http.GET();

    if (httpResponseCode == 200) {
      String payload = http.getString();
      Serial.println("[OTA] Version info payload: " + payload);

      int versionIdx = payload.indexOf("\"version\"");
      int urlIdx = payload.indexOf("\"url\"");

      if (versionIdx != -1 && urlIdx != -1) {
        int verStart = payload.indexOf(":", versionIdx) + 1;
        verStart = payload.indexOf("\"", verStart) + 1;
        int verEnd = payload.indexOf("\"", verStart);
        String remoteVersion = payload.substring(verStart, verEnd);
        remoteVersion.trim();

        int urlStart = payload.indexOf(":", urlIdx) + 1;
        urlStart = payload.indexOf("\"", urlStart) + 1;
        int urlEnd = payload.indexOf("\"", urlStart);
        String binUrl = payload.substring(urlStart, urlEnd);
        binUrl.trim();

        Serial.printf("[OTA] Current Version: %s, Remote Version: %s\n", CURRENT_VERSION.c_str(), remoteVersion.c_str());

        if (remoteVersion != CURRENT_VERSION && binUrl.length() > 0) {
          Serial.println("[OTA] New version found. Starting HTTP Update...");
          
          t_httpUpdate_return ret = httpUpdate.update(secureClient, binUrl);

          switch (ret) {
            case HTTP_UPDATE_FAILED:
              Serial.printf("[OTA] Update failed. Error (%d): %s\n", 
                            httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
              break;
            case HTTP_UPDATE_NO_UPDATES:
              Serial.println("[OTA] No updates available.");
              break;
            case HTTP_UPDATE_OK:
              Serial.println("[OTA] Update successful. Rebooting...");
              break;
          }
        } else {
          Serial.println("[OTA] Firmware is up to date.");
        }
      } else {
        Serial.println("[OTA] Parse error: JSON formatting mismatch.");
      }
    } else {
      Serial.printf("[OTA] Version check failed, HTTP Code: %d\n", httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("[OTA] Connection to version check URL failed.");
  }
}
