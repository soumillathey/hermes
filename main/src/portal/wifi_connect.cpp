/**
 * wifi_connect.cpp
 * Wi-Fi Station mode connection and auto-reconnect logic.
 */

#include "wifi_portal.h"
#include "../config/config_manager.h"
#include "../led/rgb_led.h"
#include "../led/seven_segment.h"
#include <WiFi.h>

bool connectToWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);        // Disable modem sleep for max throughput
  WiFi.setAutoReconnect(true);
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());

  displayErrorCode(SEG_CODE_WIFI_ERR); // Show '1' while connecting

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    setSevenSegmentDP(attempts % 2 == 0); // Blink DP while trying
    delay(500);
    attempts++;
  }

  setSevenSegmentDP(false);
  updateRGBStatus();
  return (WiFi.status() == WL_CONNECTED);
}

void autoReconnectWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.printf("[WiFi] Reconnecting... Status: %d\n", WiFi.status());
    WiFi.disconnect();
    WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
  }
}
