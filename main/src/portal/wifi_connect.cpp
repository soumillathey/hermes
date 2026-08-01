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
  WiFi.disconnect(true);       // Disconnect and reset STA stack cleanly
  delay(100);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);        // Disable modem sleep for max throughput & zero packet drop
  WiFi.setAutoReconnect(true);

  Serial.printf("[WiFi] Initiating STA connection to '%s'...\n", wifi_ssid.c_str());
  WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());

  displayErrorCode(SEG_CODE_WIFI_ERR); // Show '1' on 7-segment display while connecting

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) { // 20s max timeout
    setSevenSegmentDP(attempts % 2 == 0); // Blink DP while connecting
    delay(500);
    attempts++;
  }

  setSevenSegmentDP(false);

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("[WiFi] Connected! IP Address: %s | Signal (RSSI): %d dBm\n",
                  WiFi.localIP().toString().c_str(), WiFi.RSSI());
  } else {
    Serial.printf("[WiFi] Connection failed! Final status code: %d\n", WiFi.status());
  }

  updateRGBStatus();
  return (WiFi.status() == WL_CONNECTED);
}

void autoReconnectWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.printf("[WiFi] Reconnecting... Current Status: %d\n", WiFi.status());
    WiFi.reconnect();
  }
}
