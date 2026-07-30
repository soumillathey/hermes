#ifndef RGB_LED_H
#define RGB_LED_H

#include <Arduino.h>

// Set to 'true' if common pin is wired to 3.3V / VCC (Common Anode)
// Set to 'false' if common pin is wired to GND (Common Cathode)
#define IS_COMMON_ANODE true

void setupRGBLED();
void setRGBColor(bool red, bool green, bool blue);
void setRGBRed();
void setRGBGreen();
void setRGBBlue();
void triggerRGBBlue(unsigned long durationMs = 5000);
void triggerRGBGreen(unsigned long durationMs = 5000);
void updateRGBStatus();
void runRGBStartupTest();

#endif // RGB_LED_H
