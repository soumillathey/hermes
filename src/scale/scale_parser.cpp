#include "scale_parser.h"
#include "../config/config_manager.h"
#include "../network/supabase_post.h"

ScaleState scaleState = SCALE_IDLE;
static double currentStableWeightCandidate = 0.0;
static unsigned long stableStartTime = 0;
static const double STABILITY_TOLERANCE = 2.0;
static const unsigned long STABILITY_DURATION = 10000;

void processNewWeight(double parsedWeight) {
  static double lastPrintedWeight = -9999.0;
  if (abs(parsedWeight - lastPrintedWeight) >= 1.0) {
    Serial.printf("[Scale] Parsed weight: %.3f (Threshold: %.1f)\n", parsedWeight, supabase_weight_threshold);
    lastPrintedWeight = parsedWeight;
  }

  if (parsedWeight <= 0.0) {
    if (scaleState != SCALE_IDLE) {
      scaleState = SCALE_IDLE;
      stableStartTime = 0;
      currentStableWeightCandidate = 0.0;
    }
    return;
  }

  if (scaleState == SCALE_IDLE) {
    if (parsedWeight >= supabase_weight_threshold) {
      scaleState = SCALE_STABILIZING;
      stableStartTime = millis();
      currentStableWeightCandidate = parsedWeight;
    }
  } 
  else if (scaleState == SCALE_STABILIZING) {
    if (abs(parsedWeight - currentStableWeightCandidate) <= STABILITY_TOLERANCE) {
      unsigned long elapsed = millis() - stableStartTime;
      if (elapsed >= STABILITY_DURATION) {
        if (parsedWeight >= supabase_weight_threshold) {
          Serial.printf("[Scale] Weight stable reached: %.3f. Uploading to Supabase...\n", parsedWeight);
          scaleState = SCALE_STABLE_RECORDED;
          postToSupabase(parsedWeight);
        } else {
          stableStartTime = millis();
          currentStableWeightCandidate = parsedWeight;
        }
      }
    } else {
      stableStartTime = millis();
      currentStableWeightCandidate = parsedWeight;
    }
  }
}
