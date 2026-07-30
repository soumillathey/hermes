/**
 * seven_segment_driver.cpp
 * Low-level 7-segment LED GPIO driver.
 * Handles: segment bitmask lookup, pin setup, clear, char display.
 */

#include "seven_segment.h"

// Bit mapping: Bit0=a, Bit1=b, Bit2=c, Bit3=d, Bit4=e, Bit5=f, Bit6=g
static uint8_t getSegmentMask(char c) {
  switch (c) {
    case '0': return 0b00111111; // a,b,c,d,e,f
    case '1': return 0b00000110; // b,c
    case '2': return 0b01011011; // a,b,d,e,g
    case '3': return 0b01001111; // a,b,c,d,g
    case '4': return 0b01100110; // b,c,f,g
    case '5': return 0b01101101; // a,c,d,f,g
    case '6': return 0b01111101; // a,c,d,e,f,g
    case '7': return 0b00000111; // a,b,c
    case '8': return 0b01111111; // a,b,c,d,e,f,g
    case '9': return 0b01101111; // a,b,c,d,f,g
    case 'E': case 'e': return 0b01111001; // a,d,e,f,g
    default:  return 0b00000000; // All OFF
  }
}

void setupSevenSegment() {
  const uint8_t pins[] = {SEG_A, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F, SEG_G, SEG_DP};
  for (uint8_t p : pins) pinMode(p, OUTPUT);
  clearSevenSegment();
}

void clearSevenSegment() {
  const uint8_t pins[] = {SEG_A, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F, SEG_G, SEG_DP};
  uint8_t off = SEG_IS_COMMON_ANODE ? HIGH : LOW;
  for (uint8_t p : pins) digitalWrite(p, off);
}

void displaySevenSegmentChar(char c) {
  uint8_t mask = getSegmentMask(c);
  bool seg[7];
  for (int i = 0; i < 7; i++) seg[i] = (mask >> i) & 0x01;

  const uint8_t pins[] = {SEG_A, SEG_B, SEG_C, SEG_D, SEG_E, SEG_F, SEG_G};
  for (int i = 0; i < 7; i++) {
    bool on = seg[i];
    digitalWrite(pins[i], SEG_IS_COMMON_ANODE ? (on ? LOW : HIGH) : (on ? HIGH : LOW));
  }
}
