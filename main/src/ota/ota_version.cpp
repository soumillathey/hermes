/**
 * ota_version.cpp
 * Fetches and parses the version manifest from GitHub.
 * Triggers firmware download if a newer version is available.
 */

#include "ota_updater.h"
#include "ota_flash.h"
#include "../led/rgb_led.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

static const String CURRENT_VERSION = "1.0.2";
static const String OTA_VERSION_URL  =
  "https://raw.githubusercontent.com/SoumilLathey/gluvok-hardware-ota/main/firmware/version.json";

const unsigned long OTA_CHECK_INTERVAL = 3600000; // 1 hour
unsigned long lastOtaCheckTime = 0;

void checkForUpdates() {
  if (WiFi.status() != WL_CONNECTED) return;
  Serial.println("[OTA] Checking for firmware updates...");

  WiFiClientSecure secureClient;
  secureClient.setInsecure();

  HTTPClient http;
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  if (!http.begin(secureClient, OTA_VERSION_URL)) return;

  http.setTimeout(10000);
  http.addHeader("User-Agent", "ESP32");
  http.addHeader("Connection", "close");

  int code = http.GET();
  if (code != 200) {
    Serial.printf("[OTA] Version check failed, HTTP error: %d\n", code);
    http.end();
    return;
  }

  String payload = http.getString();
  int vIdx = payload.indexOf("\"version\"");
  int uIdx = payload.indexOf("\"url\"");
  if (vIdx == -1 || uIdx == -1) { http.end(); return; }

  int vStart = payload.indexOf("\"", payload.indexOf(":", vIdx) + 1) + 1;
  String remoteVersion = payload.substring(vStart, payload.indexOf("\"", vStart));
  remoteVersion.trim();

  int uStart = payload.indexOf("\"", payload.indexOf(":", uIdx) + 1) + 1;
  String binUrl = payload.substring(uStart, payload.indexOf("\"", uStart));
  binUrl.trim();
  http.end();

  if (remoteVersion != CURRENT_VERSION && binUrl.length() > 0) {
    Serial.printf("[OTA] New firmware found (%s -> %s). Starting download...\n",
                  CURRENT_VERSION.c_str(), remoteVersion.c_str());
    performOtaUpdate(binUrl, remoteVersion);
  } else {
    Serial.printf("[OTA] Firmware is up to date (v%s).\n", CURRENT_VERSION.c_str());
    updateRGBStatus();
  }
}
