#ifndef SEVEN_SEGMENT_H
#define SEVEN_SEGMENT_H

#include <Arduino.h>

// Common Anode configuration:
// Common pin is wired to 3.3V (VCC).
// GPIO LOW = Segment ON, GPIO HIGH = Segment OFF.
#define SEG_IS_COMMON_ANODE true

// GPIO Pin Definitions for ESP32 -> 7-Segment Display
#define SEG_A  14
#define SEG_B  32
#define SEG_C  33
#define SEG_D  18
#define SEG_E  22
#define SEG_F  4
#define SEG_G  23
#define SEG_DP 21

// System Error / Status Codes for 7-Segment Display
enum SevenSegCode {
  SEG_CODE_READY        = 0, // '0' - Normal Operation / Connected & Ready
  SEG_CODE_WIFI_ERR     = 1, // '1' - Wi-Fi Connection Error
  SEG_CODE_AP_MODE      = 2, // '2' - AP / Config Portal Mode Active
  SEG_CODE_AUTH_ERR     = 3, // '3' - Supabase Authentication Error
  SEG_CODE_POST_ERR     = 4, // '4' - Supabase Payload Upload Error
  SEG_CODE_SCALE_ERR    = 5, // '5' - Scale UART Data / Communication Error
  SEG_CODE_OTA_ERR      = 6, // '6' - OTA Firmware Update Error
  SEG_CODE_OTA_PROGRESS = 7, // '7' - OTA Update In Progress
  SEG_CODE_NVS_ERR      = 8, // '8' - NVS Config Load Error
  SEG_CODE_GENERAL_ERR  = 9  // 'E' - General System / Hardware Error
};

void setupSevenSegment();
void displaySevenSegmentChar(char c);
void displayErrorCode(SevenSegCode code);
void setSevenSegmentDP(bool enable);
void clearSevenSegment();
void runSevenSegmentStartupTest();

#endif // SEVEN_SEGMENT_H
