#include "scale_parser.h"
#include "../config/config_manager.h"
#include "../network/supabase_post.h"

ScaleState scaleState = SCALE_IDLE;
static const double STABILITY_TOLERANCE = 2.0;          // Max allowable variation in kg
static const unsigned long STABILITY_DURATION = 10000;  // 10 seconds (10,000 ms)

static double currentStableCandidate = 0.0;
static unsigned long candidateStartTime = 0;

void processNewWeight(double parsedWeight) {
  static double lastPrintedWeight = -9999.0;
  if (abs(parsedWeight - lastPrintedWeight) >= 0.1) {
    Serial.printf("[Scale] Parsed weight: %.3f (Threshold: %.1f)\n", parsedWeight, supabase_weight_threshold);
    lastPrintedWeight = parsedWeight;
  }

  unsigned long now = millis();

  // 1. Session End Condition: Weight returned to zero
  if (parsedWeight <= 0.0) {
    if (scaleState != SCALE_IDLE) {
      Serial.println("[Scale Session] Weight returned to zero. Session closed. Ready for next weighing.");
      scaleState = SCALE_IDLE;
      currentStableCandidate = 0.0;
      candidateStartTime = 0;
    }
    return;
  }

  // 2. Single-Transmission Lockout: Only ONE weight sent per session
  if (scaleState == SCALE_STABLE_RECORDED) {
    return; // Ignore further stable weights until zero return
  }

  // 3. Weight below min upload threshold (e.g. 50 kg)
  if (parsedWeight < supabase_weight_threshold) {
    scaleState = SCALE_IDLE;
    currentStableCandidate = 0.0;
    candidateStartTime = 0;
    return;
  }

  // 4. Active Session: Evaluate 10-second continuous stability
  if (scaleState == SCALE_IDLE) {
    scaleState = SCALE_STABILIZING;
    currentStableCandidate = parsedWeight;
    candidateStartTime = now;
    return;
  }

  if (scaleState == SCALE_STABILIZING) {
    // If current weight is within stability tolerance (+/- 2.0 kg) of candidate
    if (abs(parsedWeight - currentStableCandidate) <= STABILITY_TOLERANCE) {
      unsigned long elapsed = now - candidateStartTime;
      if (elapsed >= STABILITY_DURATION) {
        Serial.printf("[Scale Session] Stable weight confirmed (10s): %.3f. Transmitting to Supabase...\n", currentStableCandidate);
        scaleState = SCALE_STABLE_RECORDED; // Lock session until weight returns to zero
        postToSupabase(currentStableCandidate);
      }
    } else {
      // Weight shifted beyond tolerance (e.g. from 95kg to 70kg). Reset candidate & 10s timer!
      currentStableCandidate = parsedWeight;
      candidateStartTime = now;
    }
  }
}

static String lineBuffer = "";
static unsigned long lastCharTime = 0;

static void parseCurrentBuffer() {
  if (lineBuffer.length() > 0) {
    int len = lineBuffer.length();
    int i = 0;
    bool isNeg = false;
    
    // Find first sign or digit
    while (i < len) {
      char ch = lineBuffer[i];
      if (ch == '-') {
        isNeg = true;
        i++;
        break;
      } else if (ch == '+') {
        i++;
        break;
      } else if (isDigit(ch) || ch == '.') {
        break;
      }
      i++;
    }

    // Skip spaces
    while (i < len && lineBuffer[i] == ' ') {
      i++;
    }

    // Extract numeric substring (first number only)
    String numStr = "";
    while (i < len) {
      char ch = lineBuffer[i];
      if (isDigit(ch) || ch == '.') {
        numStr += ch;
      } else {
        break; // Stop at units or other trailing fields
      }
      i++;
    }

    if (numStr.length() > 0) {
      double parsedVal = numStr.toDouble();
      if (isNeg) {
        parsedVal = -parsedVal;
      }
      processNewWeight(parsedVal);
    }
    
    lineBuffer = "";
  }
}

void handleScaleChar(char c) {
  // Inter-character timeout reset (300ms gap = new packet)
  if (millis() - lastCharTime > 300 && lineBuffer.length() > 0) {
    parseCurrentBuffer();
  }
  lastCharTime = millis();

  if (c == '\n' || c == '\r' || c == 0x03 || c == 0x02) {
    parseCurrentBuffer();
  } else {
    lineBuffer += c;
    if (lineBuffer.length() >= 48) {
      parseCurrentBuffer();
    }
  }
}
