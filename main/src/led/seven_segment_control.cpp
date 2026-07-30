/**
 * seven_segment_control.cpp
 * High-level 7-segment display controller.
 * Handles: error code mapping, decimal point, boot startup test.
 */

#include "seven_segment.h"

void displayErrorCode(SevenSegCode code) {
  switch (code) {
    case SEG_CODE_READY:        displaySevenSegmentChar('0'); break; // Ready / OK
    case SEG_CODE_WIFI_ERR:     displaySevenSegmentChar('1'); break; // Wi-Fi Error
    case SEG_CODE_AP_MODE:      displaySevenSegmentChar('2'); break; // AP Config Mode
    case SEG_CODE_AUTH_ERR:     displaySevenSegmentChar('3'); break; // Auth Error
    case SEG_CODE_POST_ERR:     displaySevenSegmentChar('4'); break; // POST Upload Error
    case SEG_CODE_SCALE_ERR:    displaySevenSegmentChar('5'); break; // Scale UART Error
    case SEG_CODE_OTA_ERR:      displaySevenSegmentChar('6'); break; // OTA Firmware Error
    case SEG_CODE_OTA_PROGRESS: displaySevenSegmentChar('7'); break; // OTA In Progress
    case SEG_CODE_NVS_ERR:      displaySevenSegmentChar('8'); break; // NVS/Boot Error
    case SEG_CODE_GENERAL_ERR:  displaySevenSegmentChar('E'); break; // General Error
    default:                    clearSevenSegment(); break;
  }
}

void setSevenSegmentDP(bool enable) {
  digitalWrite(SEG_DP, SEG_IS_COMMON_ANODE ? (enable ? LOW : HIGH) : (enable ? HIGH : LOW));
}

void runSevenSegmentStartupTest() {
  Serial.println("[7-Seg] Running boot startup self-test...");
  const SevenSegCode testCodes[] = {
    SEG_CODE_READY, SEG_CODE_WIFI_ERR, SEG_CODE_AP_MODE, SEG_CODE_AUTH_ERR,
    SEG_CODE_POST_ERR, SEG_CODE_SCALE_ERR, SEG_CODE_OTA_ERR,
    SEG_CODE_OTA_PROGRESS, SEG_CODE_NVS_ERR, SEG_CODE_GENERAL_ERR
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
