#include "NavigationFeature.h"

void NavigationFeature::begin() {
  // Add GPS/UART/map setup here when the feature is promoted into src/.
  // Do not add navigation state to the stable BMS data model.
}

void NavigationFeature::loop() {
  // Keep GPS parsing and map processing non-blocking.
}

void NavigationFeature::stop() {
  // Add shutdown behavior here if the feature uses external hardware.
}

