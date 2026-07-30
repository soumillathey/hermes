/**
 * ESP32 Wi-Fi Configuration Portal & Cloud Supabase Data Logger
 * Gluvok by Lathey Weigh Trix
 *
 * Folder Structure:
 *  - config/  : Flash NVS Preferences storage manager
 *  - scale/   : UART scale stream parser & stability state machine
 *  - network/ : Supabase Auth JWT login & REST weighment payload upload
 *  - portal/  : Access Point WebServer & Station WiFi connector
 *  - ota/     : Over-The-Air firmware updater
 *  - ui/      : Embedded HTML Web Portal pages
 */

#include <WiFi.h>

#include "src/config/config_manager.h"
#include "src/ota/ota_updater.h"
#include "src/scale/scale_parser.h"
#include "src/network/supabase_auth.h"
#include "src/network/supabase_post.h"
#include "src/portal/wifi_portal.h"
#include "src/led/rgb_led.h"
#include "src/led/seven_segment.h"

HardwareSerial Indicator(2);

static unsigned long lastWifiCheckTime = 0;

void setup() {
  setupRGBLED();
  setupSevenSegment();

  Serial.begin(115200);
  Serial.println("\n==============================================");
  Serial.println("Gluvok by Lathey Weigh Trix Starting...");
  Serial.printf("Device MAC Address: %s\n", WiFi.macAddress().c_str());
  Serial.println("==============================================");

  runSevenSegmentStartupTest();
  Indicator.begin(1200, SERIAL_8N1, 16, 17);
  clearWifiCredentials();
  loadSettings();

  if (wifi_ssid.length() > 0) {
    Serial.printf("[WiFi] Attempting to connect to saved Wi-Fi: %s...\n", wifi_ssid.c_str());
    if (connectToWiFi()) {
      currentState = STATE_CONNECTED_NORMAL;
      Serial.println("[WiFi] Connected successfully on boot!");
      checkForUpdates();
    } else {
      Serial.println("[WiFi] Saved connection failed. Starting configuration portal...");
      startAPMode();
    }
  } else {
    Serial.println("[WiFi] No saved credentials found. Starting configuration portal...");
    startAPMode();
  }

  lastWifiCheckTime = millis();
}

void loop() {
  server.handleClient();
  updateRGBStatus();

  while (Indicator.available()) {
    char c = (char)Indicator.read();
    handleScaleChar(c);
  }

  if (currentState == STATE_CONNECTED_NORMAL) {
    unsigned long currentMillis = millis();

    if (currentMillis - lastWifiCheckTime >= WIFI_CHECK_INTERVAL) {
      lastWifiCheckTime = currentMillis;
      autoReconnectWiFi();
    }

    if (currentMillis - lastOtaCheckTime >= OTA_CHECK_INTERVAL) {
      lastOtaCheckTime = currentMillis;
      checkForUpdates();
    }
  }

  delay(10);
}