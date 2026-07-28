#ifndef SCALE_PARSER_H
#define SCALE_PARSER_H

#include <Arduino.h>

enum ScaleState {
  SCALE_IDLE,
  SCALE_STABILIZING,
  SCALE_STABLE_RECORDED
};

extern ScaleState scaleState;
extern double currentStableWeightCandidate;
extern unsigned long stableStartTime;

const double STABILITY_TOLERANCE = 2.0;         // Allowed weight fluctuation range
const unsigned long STABILITY_DURATION = 10000; // Time required for weight stabilization (ms)

void processNewWeight(double parsedWeight);

#endif // SCALE_PARSER_H
