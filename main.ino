/**
 * ESP32 Wi-Fi Configuration Portal & Cloud Supabase Data Logger
 * Gluvok by Lathey Weigh Trix
 *
 * Folder Structure:
 *  - src/config/  : Flash NVS Preferences storage manager
 *  - src/scale/   : UART scale stream parser & stability state machine
 *  - src/network/ : Supabase Auth JWT login & REST weighment payload upload
 *  - src/portal/  : Access Point WebServer & Station WiFi connector
 *  - src/ota/     : Over-The-Air firmware updater
 *  - src/ui/      : Embedded HTML Web Portal pages
 */

#include "src/config/config_manager.h"
#include "src/ota/ota_updater.h"
#include "src/scale/scale_parser.h"
#include "src/network/supabase_auth.h"
#include "src/network/supabase_post.h"
#include "src/portal/wifi_portal.h"

HardwareSerial Indicator(2);

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

  Indicator.begin(1200, SERIAL_8N1, 16, 17);
  loadSettings();
  startAPMode();

  lastWifiCheckTime = millis();
}

void loop() {
  server.handleClient();

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