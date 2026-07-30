#include "rgb_led.h"
#include "seven_segment.h"
#include "../portal/wifi_portal.h"
#include <WiFi.h>

#define RED_PIN 25
#define GREEN_PIN 26
#define BLUE_PIN 27

static String lastColor = "";
static unsigned long blueUntilMillis = 0;
static unsigned long greenUntilMillis = 0;

void triggerRGBBlue(unsigned long durationMs) {
  blueUntilMillis = millis() + durationMs;
  setRGBBlue();
}

void triggerRGBGreen(unsigned long durationMs) {
  blueUntilMillis = 0; // Clear blue timer so green takes 100% priority!
  greenUntilMillis = millis() + durationMs;
  setRGBGreen();
}

void setupRGBLED() {
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  runRGBStartupTest();
}

void runRGBStartupTest() {
  Serial.println("[RGB] Boot Self-Test: RED -> GREEN -> BLUE");
  setRGBRed();
  delay(400);
  setRGBGreen();
  delay(400);
  setRGBBlue();
  delay(400);
}

void setRGBColor(bool red, bool green, bool blue) {
  // Common Anode (Common pin to 3.3V): LOW = ON, HIGH = OFF
  // Common Cathode (Common pin to GND): HIGH = ON, LOW = OFF
  bool redLevel   = IS_COMMON_ANODE ? !red   : red;
  bool greenLevel = IS_COMMON_ANODE ? !green : green;
  bool blueLevel  = IS_COMMON_ANODE ? !blue  : blue;

  digitalWrite(RED_PIN, redLevel ? HIGH : LOW);
  digitalWrite(GREEN_PIN, greenLevel ? HIGH : LOW);
  digitalWrite(BLUE_PIN, blueLevel ? HIGH : LOW);
}

void setRGBRed() {
  if (lastColor != "RED") {
    lastColor = "RED";
    Serial.println("[RGB] Status LED -> RED");
  }
  setRGBColor(true, false, false);
}

void setRGBGreen() {
  if (lastColor != "GREEN") {
    lastColor = "GREEN";
    Serial.println("[RGB] Status LED -> GREEN");
  }
  setRGBColor(false, true, false);
}

void setRGBBlue() {
  if (lastColor != "BLUE") {
    lastColor = "BLUE";
    Serial.println("[RGB] Status LED -> BLUE");
  }
  setRGBColor(false, false, true);
}

void updateRGBStatus() {
  if (millis() < greenUntilMillis) {
    setRGBGreen();
    return;
  }
  if (millis() < blueUntilMillis) {
    setRGBBlue();
    return;
  }

  if (WiFi.status() == WL_CONNECTED) {
    setRGBBlue(); // Default connected color is BLUE
    displayErrorCode(SEG_CODE_READY);
  } else {
    setRGBRed();
    if (currentState == STATE_AP_MODE) {
      displayErrorCode(SEG_CODE_AP_MODE);
    } else {
      displayErrorCode(SEG_CODE_WIFI_ERR);
    }
  }
}

