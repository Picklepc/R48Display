#pragma once

#include <FeatureModule.h>

// Optional feature template. This file is intentionally outside src/ so SD
// support is not compiled into the stable firmware by default.
class SdCardFeature : public FeatureModule {
 public:
  const char *id() const override { return "sd_card"; }
  const char *label() const override { return "SD card storage"; }

  void begin() override;
  void loop() override;
  void stop() override;
};

