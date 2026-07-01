#pragma once
#include <stdint.h>
#include <lvgl.h>

// Animation type IDs — stored in ThemeProfile.animType and passed through Snapshot.animType.
namespace Anim {
  constexpr uint8_t None          = 0;
  constexpr uint8_t Stars         = 1;   // subtle space twinkle
  constexpr uint8_t Embers        = 2;   // floating orange sparks
  constexpr uint8_t Lightning     = 3;   // purple flash strikes
  constexpr uint8_t Ripple        = 4;   // water ripple bursts
  constexpr uint8_t SpeedLines    = 5;   // red horizontal streaks
  constexpr uint8_t Rave          = 6;   // LOUD neon particle storm
  constexpr uint8_t PixelRain     = 7;   // retro matrix fall
  constexpr uint8_t Geometry      = 8;   // slow-drifting shapes
  constexpr uint8_t FireworksFlag = 9;   // 'Murica flag + fireworks
  constexpr uint8_t Bats          = 10;  // halloween bats gliding
  constexpr uint8_t Snow          = 11;  // christmas snowfall
  constexpr uint8_t Leaves        = 12;  // autumn leaves
  constexpr uint8_t Hearts        = 13;  // valentine hearts rising
  constexpr uint8_t Grass         = 14;  // mower grass clippings
  constexpr uint8_t Fireworks     = 15;  // new year's bursts, no flag
}

namespace AnimLayer {
  // Call at the START of buildPage (before any UI widgets) so animation objects
  // are z-ordered behind the dashboard. lv_obj_clean() on the screen destroys
  // the previous pool; this rebuilds it fresh.
  void build(lv_obj_t *root, uint8_t animType, bool enabled);

  // Advance particle physics and update LVGL object positions / opacity.
  // Call from DisplayUi::tick() on every loop.
  void step(uint32_t nowMs);

  // Enable / disable without rebuilding the page.
  // Hides all active particles immediately when disabled.
  void setEnabled(bool enabled);
}
