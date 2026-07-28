#include "scale_parser.h"
#include "config_manager.h"
#include "supabase_client.h"

ScaleState scaleState = SCALE_IDLE;
double currentStableWeightCandidate = 0.0;
unsigned long stableStartTime = 0;

void processNewWeight(double parsedWeight) {
  static double lastPrintedWeight = -9999.0;
  if (abs(parsedWeight - lastPrintedWeight) >= 1.0) {
    Serial.printf("[Scale] Parsed weight: %.3f (Threshold: %.1f)\n", parsedWeight, supabase_weight_threshold);
    lastPrintedWeight = parsedWeight;
  }

  // The session only closes when weight returns to zero (<= 0.0)
  if (parsedWeight <= 0.0) {
    if (scaleState != SCALE_IDLE) {
      scaleState = SCALE_IDLE;
      stableStartTime = 0;
      currentStableWeightCandidate = 0.0;
    }
    return;
  }

  // Weight is positive (> 0.0)
  if (scaleState == SCALE_IDLE) {
    // The session must begin when the threshold is crossed (>= supabase_weight_threshold)
    if (parsedWeight >= supabase_weight_threshold) {
      scaleState = SCALE_STABILIZING;
      stableStartTime = millis();
      currentStableWeightCandidate = parsedWeight;
    }
  } 
  else if (scaleState == SCALE_STABILIZING) {
    // Session is active. Check stability of the weight.
    if (abs(parsedWeight - currentStableWeightCandidate) <= STABILITY_TOLERANCE) {
      unsigned long elapsed = millis() - stableStartTime;
      if (elapsed >= STABILITY_DURATION) {
        if (parsedWeight >= supabase_weight_threshold) {
          Serial.printf("[Scale] Weight stable reached: %.3f. Uploading to Supabase...\n", parsedWeight);
          scaleState = SCALE_STABLE_RECORDED;
          postToSupabase(parsedWeight);
        } else {
          // Stabilized below threshold, reset timer to prevent immediate upload if it rises again
          stableStartTime = millis();
          currentStableWeightCandidate = parsedWeight;
        }
      }
    } else {
      stableStartTime = millis();
      currentStableWeightCandidate = parsedWeight;
    }
  }
  else if (scaleState == SCALE_STABLE_RECORDED) {
    // Already recorded. Session remains active until weight returns to zero (<= 0.0), handled above.
  }
}
