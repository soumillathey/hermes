/**
 * scale_uart.cpp
 * UART2 serial stream reader and line parser for the weighing scale indicator.
 * Handles: character buffering, inter-packet timeout flush, numeric extraction.
 */

#include "scale_parser.h"

static String lineBuffer  = "";
static unsigned long lastCharTime = 0;

static void parseCurrentBuffer() {
  if (lineBuffer.length() == 0) return;

  int len = lineBuffer.length();
  int i   = 0;
  bool isNeg = false;

  // Find first sign or digit
  while (i < len) {
    char ch = lineBuffer[i];
    if      (ch == '-') { isNeg = true; i++; break; }
    else if (ch == '+') { i++;              break; }
    else if (isDigit(ch) || ch == '.') break;
    i++;
  }

  while (i < len && lineBuffer[i] == ' ') i++; // Skip spaces

  // Extract numeric substring
  String numStr = "";
  while (i < len) {
    char ch = lineBuffer[i];
    if (isDigit(ch) || ch == '.') numStr += ch;
    else break;
    i++;
  }

  if (numStr.length() > 0) {
    double val = numStr.toDouble();
    if (isNeg) val = -val;
    processNewWeight(val);
  }

  lineBuffer = "";
}

void handleScaleChar(char c) {
  // 300ms inter-character gap flushes any incomplete packet
  if (millis() - lastCharTime > 300 && lineBuffer.length() > 0) {
    parseCurrentBuffer();
  }
  lastCharTime = millis();

  if (c == '\n' || c == '\r' || c == 0x03 || c == 0x02) {
    parseCurrentBuffer();                         // Standard and STX/ETX terminators
  } else {
    lineBuffer += c;
    if (lineBuffer.length() >= 48) parseCurrentBuffer(); // Overflow safety
  }
}
