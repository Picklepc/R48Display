#pragma once

#include <Arduino_GFX_Library.h>

// ST77916 panel driver using Waveshare's own vendor-tuned register table
// (voltage/gamma settings) instead of the generic table bundled with
// GFX Library for Arduino. The stock Arduino_ST77916 init sequence produced
// a corrupted/noisy image on this panel batch; Waveshare's official
// ESP32-S3-Touch-LCD-1.85C demo (Display_ST77916.cpp, "case 2" vendor
// table) uses different B0-D9 voltage/gamma values that match this
// hardware revision.
class Arduino_ST77916_R48 : public Arduino_ST77916 {
public:
  Arduino_ST77916_R48(
      Arduino_DataBus *bus, int8_t rst, uint8_t r,
      bool ips, int16_t w, int16_t h);

protected:
  void tftInit() override;
};
