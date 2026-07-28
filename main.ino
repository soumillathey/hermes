/**
 * ESP32 Wi-Fi Configuration Portal & Cloud Supabase Data Logger
 * Gluvok by Lathey Weigh Trix
 *
 * Modularized Architecture:
 *  - html_pages.h: Glassmorphic HTML web UI templates
 *  - config_manager.h/.cpp: Non-Volatile Flash (NVS Preferences) storage
 *  - scale_parser.h/.cpp: UART indicator parsing & weight stability state machine
 *  - supabase_client.h/.cpp: Supabase Auth JWT login & REST weighment upload
 *  - wifi_portal.h/.cpp: AP mode, station connection, WebServer routes
 *  - ota_updater.h/.cpp: Over-The-Air firmware updates
 */

#include "config_manager.h"
#include "ota_updater.h"
#include "scale_parser.h"
#include "supabase_client.h"
#include "wifi_portal.h"

// Hardware Serial object for scale indicator
HardwareSerial Indicator(2);

// Stream parsing state
static String digitBuffer = "";
static bool isNegative = false;
static unsigned long lastWifiCheckTime = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n==============================================");
  Serial.println("Gluvok by Lathey Weigh Trix Starting...");
  Serial.printf("Device MAC Address: %s\n", WiFi.macAddress().c_str());
  Serial.println("==============================================");

  // Initialize UART2 for Weight Indicator (1200 Baud 8N1, RX=16, TX=17)
  Indicator.begin(1200, SERIAL_8N1, 16, 17);
  Serial.println("Indicator listening on UART2 (pins 16/RX, 17/TX) at 1200 baud...");

  // Load configured settings from non-volatile flash storage
  loadSettings();

  // Force AP Config Mode on boot
  Serial.println("Forcing AP Config Mode on startup.");
  startAPMode();

  lastWifiCheckTime = millis();
}

void loop() {
  // Handle Web Server requests
  server.handleClient();

  // Read scale indicator UART stream
  while (Indicator.available()) {
    char c = (char)Indicator.read();
    
    if (c == '-') {
      isNegative = true;
      digitBuffer = "";
    } else if (isDigit(c) || c == '.') {
      digitBuffer += c;
    } else {
      if (digitBuffer.length() >= 1 && digitBuffer.length() <= 10) {
        double parsedVal = digitBuffer.toDouble();
        if (isNegative) {
          parsedVal = -parsedVal;
        }
        processNewWeight(parsedVal);
      }
      digitBuffer = "";
      isNegative = false;
    }
  }

  // Normal operational tasks
  if (currentState == STATE_CONNECTED_NORMAL) {
    unsigned long currentMillis = millis();

    // WiFi auto-reconnect check
    if (currentMillis - lastWifiCheckTime >= WIFI_CHECK_INTERVAL) {
      lastWifiCheckTime = currentMillis;
      autoReconnectWiFi();
    }

    // OTA update check
    if (currentMillis - lastOtaCheckTime >= OTA_CHECK_INTERVAL) {
      lastOtaCheckTime = currentMillis;
      checkForUpdates();
    }
  }

  delay(10);
}