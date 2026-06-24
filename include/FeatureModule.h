#pragma once

#include <Arduino.h>

// Lightweight interface for optional modules. Stable firmware should keep
// optional features out of the main loop until they are proven and registered
// deliberately by the maintainer.
class FeatureModule {
 public:
  virtual ~FeatureModule() = default;
  virtual const char *id() const = 0;
  virtual const char *label() const = 0;
  virtual void begin() {}
  virtual void loop() {}
  virtual void stop() {}
};

