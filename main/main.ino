/**
 * main.ino — Gluvok by Lathey Weigh Trix
 * ESP32 Wi-Fi Configuration Portal & Cloud Supabase Data Logger
 *
 * src/config/   — NVS Flash settings manager
 * src/led/      — RGB LED driver/control, 7-Segment driver/control
 * src/network/  — Supabase login, profile resolver, payload POST
 * src/portal/   — AP mode, Wi-Fi connect, config form handler
 * src/scale/    — UART2 stream reader, 10s stability state machine
 * src/ota/      — GitHub version checker, firmware flash executor
 * src/ui/       — Embedded HTML config and save pages
 */

#include <WiFi.h>
#include "src/config/config_manager.h"
#include "src/led/rgb_led.h"
#include "src/led/seven_segment.h"
#include "src/portal/wifi_portal.h"
#include "src/network/supabase_auth.h"
#include "src/network/supabase_post.h"
#include "src/scale/scale_parser.h"
#include "src/ota/ota_updater.h"

HardwareSerial Indicator(2);
static unsigned long lastWifiCheckTime = 0;

void setup() {
  setupRGBLED();        // GPIO init + RED->GREEN->BLUE startup test
  setupSevenSegment();  // GPIO init + 0->E startup test

  Serial.begin(115200);
  Serial.println("\n==============================================");
  Serial.println("Gluvok by Lathey Weigh Trix Starting...");
  Serial.printf("Device MAC Address: %s\n", WiFi.macAddress().c_str());
  Serial.println("==============================================");

  runSevenSegmentStartupTest();
  Indicator.begin(1200, SERIAL_8N1, 16, 17); // Scale UART2: RX=16, TX=17

  // clearWifiCredentials(); // Commented out so saved credentials persist
  loadSettings();

  if (wifi_ssid.length() > 0) {
    Serial.printf("[WiFi] Connecting to saved network: %s\n", wifi_ssid.c_str());
    if (connectToWiFi()) {
      currentState = STATE_CONNECTED_NORMAL;
      Serial.println("[WiFi] Connected successfully on boot!");
      checkForUpdates();
    } else {
      startAPMode();
    }
  } else {
    Serial.println("[WiFi] No saved credentials. Starting configuration portal...");
    startAPMode();
  }

  lastWifiCheckTime = millis();
}

void loop() {
  server.handleClient();
  updateRGBStatus();

  while (Indicator.available()) {
    handleScaleChar((char)Indicator.read());
  }

  if (currentState == STATE_CONNECTED_NORMAL) {
    unsigned long now = millis();
    if (now - lastWifiCheckTime >= WIFI_CHECK_INTERVAL) { lastWifiCheckTime = now; autoReconnectWiFi(); }
    if (now - lastOtaCheckTime  >= OTA_CHECK_INTERVAL)  { lastOtaCheckTime  = now; checkForUpdates();  }
  }

  delay(10);
}