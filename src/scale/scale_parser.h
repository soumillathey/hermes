#ifndef SCALE_PARSER_H
#define SCALE_PARSER_H

#include <Arduino.h>

enum ScaleState {
  SCALE_IDLE,
  SCALE_STABILIZING,
  SCALE_STABLE_RECORDED
};

extern ScaleState scaleState;

void processNewWeight(double parsedWeight);

#endif // SCALE_PARSER_H
