/**
 * rgb_led_driver.cpp
 * Low-level GPIO driver for the RGB status LED.
 * Handles: pin setup, color mixing, individual color setters.
 */

#include "rgb_led.h"

static String lastColor = "";

void setupRGBLED() {
  pinMode(RGB_RED_PIN,   OUTPUT);
  pinMode(RGB_GREEN_PIN, OUTPUT);
  pinMode(RGB_BLUE_PIN,  OUTPUT);
  runRGBStartupTest();
}

void setRGBColor(bool red, bool green, bool blue) {
  // Common Anode: LOW = ON | Common Cathode: HIGH = ON
  bool r = IS_COMMON_ANODE ? !red   : red;
  bool g = IS_COMMON_ANODE ? !green : green;
  bool b = IS_COMMON_ANODE ? !blue  : blue;
  digitalWrite(RGB_RED_PIN,   r ? HIGH : LOW);
  digitalWrite(RGB_GREEN_PIN, g ? HIGH : LOW);
  digitalWrite(RGB_BLUE_PIN,  b ? HIGH : LOW);
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
