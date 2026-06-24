#pragma once

#include <Arduino.h>

namespace R48Mic {

struct Snapshot {
  bool enabled = false;
  bool ready = false;
  bool tone = false;
  uint32_t rms = 0;
  uint32_t peak = 0;
  uint32_t floor = 220;
  uint32_t detections = 0;
  uint32_t errors = 0;
  uint32_t lastReadMs = 0;
  float threshold = 900.0f;
  String status = "mic disabled";
};

void begin(bool enabled, float threshold);
void configure(bool enabled, float threshold);
void loop(bool packCurrentActive);
bool toneDetected();
Snapshot snapshot();

}  // namespace R48Mic

