#include "SdCardFeature.h"

void SdCardFeature::begin() {
  // Add SD initialization here when the feature is promoted into src/.
  // Keep all card mount/retry/state logic local to this module.
}

void SdCardFeature::loop() {
  // Add low-frequency maintenance here. Avoid blocking the BMS/display loop.
}

void SdCardFeature::stop() {
  // Add clean unmount/disable behavior here if needed.
}

