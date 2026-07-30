#include "seven_segment.h"

// Segment bitmasks (a, b, c, d, e, f, g)
// Bit 0 = a, Bit 1 = b, Bit 2 = c, Bit 3 = d, Bit 4 = e, Bit 5 = f, Bit 6 = g
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
    case 'A': case 'a': return 0b01110111; // a,b,c,e,f,g
    case 'B': case 'b': return 0b01111100; // c,d,e,f,g
    case 'C': case 'c': return 0b00111001; // a,d,e,f
    case 'D': case 'd': return 0b01011110; // b,c,d,e,g
    case 'E': case 'e': return 0b01111001; // a,d,e,f,g
    case 'F': case 'f': return 0b01110001; // a,e,f,g
    case 'u': case 'U': return 0b00111100; // c,d,e
    case 'o': case 'O': return 0b01011100; // c,d,e,g
    case '-': return 0b01000000;          // g
    case '_': return 0b00001000;          // d
    default:  return 0b00000000;          // All segments OFF
  }
}

void setupSevenSegment() {
  pinMode(SEG_A, OUTPUT);
  pinMode(SEG_B, OUTPUT);
  pinMode(SEG_C, OUTPUT);
  pinMode(SEG_D, OUTPUT);
  pinMode(SEG_E, OUTPUT);
  pinMode(SEG_F, OUTPUT);
  pinMode(SEG_G, OUTPUT);
  pinMode(SEG_DP, OUTPUT);

  clearSevenSegment();
}

void clearSevenSegment() {
  // Common Anode (VCC connected): HIGH = OFF, LOW = ON
  digitalWrite(SEG_A, SEG_IS_COMMON_ANODE ? HIGH : LOW);
  digitalWrite(SEG_B, SEG_IS_COMMON_ANODE ? HIGH : LOW);
  digitalWrite(SEG_C, SEG_IS_COMMON_ANODE ? HIGH : LOW);
  digitalWrite(SEG_D, SEG_IS_COMMON_ANODE ? HIGH : LOW);
  digitalWrite(SEG_E, SEG_IS_COMMON_ANODE ? HIGH : LOW);
  digitalWrite(SEG_F, SEG_IS_COMMON_ANODE ? HIGH : LOW);
  digitalWrite(SEG_G, SEG_IS_COMMON_ANODE ? HIGH : LOW);
  digitalWrite(SEG_DP, SEG_IS_COMMON_ANODE ? HIGH : LOW);
}

void displaySevenSegmentChar(char c) {
  uint8_t mask = getSegmentMask(c);

  bool a     = (mask >> 0) & 0x01;
  bool b     = (mask >> 1) & 0x01;
  bool c_seg = (mask >> 2) & 0x01;
  bool d     = (mask >> 3) & 0x01;
  bool e     = (mask >> 4) & 0x01;
  bool f     = (mask >> 5) & 0x01;
  bool g     = (mask >> 6) & 0x01;

  digitalWrite(SEG_A, SEG_IS_COMMON_ANODE ? (a ? LOW : HIGH) : (a ? HIGH : LOW));
  digitalWrite(SEG_B, SEG_IS_COMMON_ANODE ? (b ? LOW : HIGH) : (b ? HIGH : LOW));
  digitalWrite(SEG_C, SEG_IS_COMMON_ANODE ? (c_seg ? LOW : HIGH) : (c_seg ? HIGH : LOW));
  digitalWrite(SEG_D, SEG_IS_COMMON_ANODE ? (d ? LOW : HIGH) : (d ? HIGH : LOW));
  digitalWrite(SEG_E, SEG_IS_COMMON_ANODE ? (e ? LOW : HIGH) : (e ? HIGH : LOW));
  digitalWrite(SEG_F, SEG_IS_COMMON_ANODE ? (f ? LOW : HIGH) : (f ? HIGH : LOW));
  digitalWrite(SEG_G, SEG_IS_COMMON_ANODE ? (g ? LOW : HIGH) : (g ? HIGH : LOW));
}

void displayErrorCode(SevenSegCode code) {
  switch (code) {
    case SEG_CODE_READY:        displaySevenSegmentChar('0'); break; // '0' (Ready / OK)
    case SEG_CODE_WIFI_ERR:     displaySevenSegmentChar('1'); break; // '1' (Wi-Fi Error)
    case SEG_CODE_AP_MODE:      displaySevenSegmentChar('2'); break; // '2' (AP Config Mode)
    case SEG_CODE_AUTH_ERR:     displaySevenSegmentChar('3'); break; // '3' (Auth Error)
    case SEG_CODE_POST_ERR:     displaySevenSegmentChar('4'); break; // '4' (Post Upload Error)
    case SEG_CODE_SCALE_ERR:    displaySevenSegmentChar('5'); break; // '5' (Scale UART Error)
    case SEG_CODE_OTA_ERR:      displaySevenSegmentChar('6'); break; // '6' (Firmware Error)
    case SEG_CODE_OTA_PROGRESS: displaySevenSegmentChar('7'); break; // '7' (Updating)
    case SEG_CODE_NVS_ERR:      displaySevenSegmentChar('8'); break; // '8' (Boot/NVS Error)
    case SEG_CODE_GENERAL_ERR:  displaySevenSegmentChar('E'); break; // 'E' (General Error)
    default:                    clearSevenSegment(); break;
  }
}

void setSevenSegmentDP(bool enable) {
  digitalWrite(SEG_DP, SEG_IS_COMMON_ANODE ? (enable ? LOW : HIGH) : (enable ? HIGH : LOW));
}

void runSevenSegmentStartupTest() {
  Serial.println("[7-Seg] Running boot startup self-test protocol...");
  SevenSegCode testCodes[] = {
    SEG_CODE_READY,        // '0'
    SEG_CODE_WIFI_ERR,     // '1'
    SEG_CODE_AP_MODE,      // '2'
    SEG_CODE_AUTH_ERR,     // '3'
    SEG_CODE_POST_ERR,     // '4'
    SEG_CODE_SCALE_ERR,    // '5'
    SEG_CODE_OTA_ERR,      // '6'
    SEG_CODE_OTA_PROGRESS, // '7'
    SEG_CODE_NVS_ERR,      // '8'
    SEG_CODE_GENERAL_ERR   // 'E'
  };

  for (int i = 0; i < 10; i++) {
    displayErrorCode(testCodes[i]);
    setSevenSegmentDP(i % 2 == 0);
    delay(250);
  }

  setSevenSegmentDP(false);
  clearSevenSegment();
  delay(100);
}
