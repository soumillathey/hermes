/**
 * rgb_led_control.cpp
 * High-level RGB LED state controller.
 * Handles: non-blocking timed triggers, startup test, status-driven display.
 */

#include "rgb_led.h"
#include "seven_segment.h"
#include "../portal/wifi_portal.h"
#include <WiFi.h>

static unsigned long blueUntilMillis  = 0;
static unsigned long greenUntilMillis = 0;

void runRGBStartupTest() {
  Serial.println("[RGB] Boot Self-Test: RED -> GREEN -> BLUE");
  setRGBRed();   delay(400);
  setRGBGreen(); delay(400);
  setRGBBlue();  delay(400);
}

void triggerRGBBlue(unsigned long durationMs) {
  blueUntilMillis = millis() + durationMs;
  setRGBBlue();
}

void triggerRGBGreen(unsigned long durationMs) {
  blueUntilMillis  = 0; // Green takes full priority
  greenUntilMillis = millis() + durationMs;
  setRGBGreen();
}

void updateRGBStatus() {
  if (millis() < greenUntilMillis) { setRGBGreen(); return; }
  if (millis() < blueUntilMillis)  { setRGBBlue();  return; }

  if (WiFi.status() == WL_CONNECTED) {
    setRGBBlue();
    displayErrorCode(SEG_CODE_READY);
  } else {
    setRGBRed();
    SevenSegCode code = (currentState == STATE_AP_MODE) ? SEG_CODE_AP_MODE : SEG_CODE_WIFI_ERR;
    displayErrorCode(code);
  }
}
