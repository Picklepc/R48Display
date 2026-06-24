#pragma once

#include <FeatureModule.h>

// Optional feature template. This file is intentionally outside src/ so GPS,
// maps, and navigation experiments do not affect the stable battery display.
class NavigationFeature : public FeatureModule {
 public:
  const char *id() const override { return "navigation"; }
  const char *label() const override { return "Navigation and mapping"; }

  void begin() override;
  void loop() override;
  void stop() override;
};

