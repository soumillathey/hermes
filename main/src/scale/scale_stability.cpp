/**
 * scale_stability.cpp
 * Weight session state machine with 10-second continuous stability detection.
 * One transmission per session; resets only on weight returning to zero.
 */

#include "scale_parser.h"
#include "../config/config_manager.h"
#include "../network/supabase_post.h"

ScaleState scaleState = SCALE_IDLE;
static const double        STABILITY_TOLERANCE = 2.0;   // ±2.0 kg allowed variation
static const unsigned long STABILITY_DURATION  = 10000; // 10 seconds

static double        currentStableCandidate = 0.0;
static unsigned long candidateStartTime     = 0;

void processNewWeight(double parsedWeight) {
  static double lastPrintedWeight = -9999.0;
  if (abs(parsedWeight - lastPrintedWeight) >= 0.1) {
    Serial.printf("[Scale] Parsed weight: %.3f (Threshold: %.1f)\n",
                  parsedWeight, supabase_weight_threshold);
    lastPrintedWeight = parsedWeight;
  }

  unsigned long now = millis();

  // Session end: weight returned to zero
  if (parsedWeight <= 0.0) {
    if (scaleState != SCALE_IDLE) {
      Serial.println("[Scale Session] Weight returned to zero. Session closed. Ready for next weighing.");
      scaleState = SCALE_IDLE;
      currentStableCandidate = 0.0;
      candidateStartTime = 0;
    }
    return;
  }

  // Lockout: only one upload per session
  if (scaleState == SCALE_STABLE_RECORDED) return;

  // Below threshold: reset
  if (parsedWeight < supabase_weight_threshold) {
    scaleState = SCALE_IDLE;
    currentStableCandidate = 0.0;
    candidateStartTime = 0;
    return;
  }

  // Start stability timer on new session
  if (scaleState == SCALE_IDLE) {
    scaleState = SCALE_STABILIZING;
    currentStableCandidate = parsedWeight;
    candidateStartTime = now;
    return;
  }

  // Evaluate 10-second stability window
  if (abs(parsedWeight - currentStableCandidate) <= STABILITY_TOLERANCE) {
    if (now - candidateStartTime >= STABILITY_DURATION) {
      Serial.printf("[Scale Session] Stable weight confirmed (10s): %.3f. Transmitting...\n",
                    currentStableCandidate);
      scaleState = SCALE_STABLE_RECORDED;
      postToSupabase(currentStableCandidate);
    }
  } else {
    // Weight shifted — reset candidate and timer
    currentStableCandidate = parsedWeight;
    candidateStartTime = now;
  }
}
