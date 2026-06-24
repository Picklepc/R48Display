#pragma once

#include <Arduino.h>
#include <Arduino_GFX_Library.h>

namespace R48DisplayUi {

static constexpr uint8_t PAGE_COUNT = 4;

struct Snapshot {
  uint8_t page = 0;
  bool fullRedraw = false;

  bool socValid = false;
  uint16_t soc = 0;
  float packVoltage = 0.0f;
  float deltaCellMv = 0.0f;
  float loadPercent = 0.0f;
  float dischargeWatts = 0.0f;
  float dischargeAmps = 0.0f;
  float chargeWatts = 0.0f;
  float chargeAmps = 0.0f;
  float healthPercent = 0.0f;
  float remainingAh = 0.0f;
  float designAh = 0.0f;
  String mode;
  String runEta;
  String chargeEta;
  String rideTime;
  String temp;
  String runtimeHours;

  bool wifiConnected = false;
  bool provisioning = false;
  int32_t wifiRssi = 0;
  String ip;
  String ssid;

  String bleLink;
  int bmsRssi = 0;
  String bmsTarget;
  String bmsAddress;
  String lastError;

  String screenBattery;
  bool touchReady = false;
  String localTime;
  String uptime;
  String firmware;

  uint32_t themeBg = 0x050605;
  uint32_t themePanel = 0x10170D;
  uint32_t themePanel2 = 0x17220F;
  uint32_t themeLine = 0x2A3524;
  uint32_t themeText = 0xF6FAF0;
  uint32_t themeMuted = 0x9CAA92;
  uint32_t themePrimary = 0xB7F23B;
  uint32_t themePrimaryDark = 0x4F7B13;
  uint32_t themeAccent = 0x40E0D0;
  uint32_t themeWarn = 0xFFD166;
  uint32_t themeBad = 0xFF4D4D;
};

void begin(Arduino_GFX *display);
void draw(const Snapshot &s);
void tick();

}  // namespace R48DisplayUi

