#include "ota_updater.h"
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

  WiFiClientSecure secureClient;
  secureClient.setInsecure();

  HTTPClient http;
  if (http.begin(secureClient, OTA_VERSION_URL)) {
    http.setTimeout(10000);
    int httpResponseCode = http.GET();

    if (httpResponseCode == 200) {
      String payload = http.getString();
      int versionIdx = payload.indexOf("\"version\"");
      int urlIdx = payload.indexOf("\"url\"");

      if (versionIdx != -1 && urlIdx != -1) {
        int verStart = payload.indexOf("\"", payload.indexOf(":", versionIdx) + 1) + 1;
        int verEnd = payload.indexOf("\"", verStart);
        String remoteVersion = payload.substring(verStart, verEnd); remoteVersion.trim();

        int urlStart = payload.indexOf("\"", payload.indexOf(":", urlIdx) + 1) + 1;
        int urlEnd = payload.indexOf("\"", urlStart);
        String binUrl = payload.substring(urlStart, urlEnd); binUrl.trim();

        if (remoteVersion != CURRENT_VERSION && binUrl.length() > 0) {
          Serial.println("[OTA] Updating firmware...");
          httpUpdate.update(secureClient, binUrl);
        }
      }
    }
    http.end();
  }
}
