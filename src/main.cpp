#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <Update.h>
#include <WebServer.h>
#include <WiFi.h>
#include <Wire.h>
#include <math.h>
#include <time.h>

#include <esp_heap_caps.h>
#include <esp_wifi.h>

#include <Arduino_GFX_Library.h>
#include <NimBLEDevice.h>

#include "BoardConfig.h"
#include "DisplayUi.h"
#include "MicDetector.h"
#include "WebPages.h"

namespace {

constexpr uint16_t COLOR_BG = 0x0000;
constexpr uint16_t COLOR_GREEN = 0x07E0;
constexpr uint16_t COLOR_YELLOW = 0xFFE0;

constexpr uint32_t WIFI_RETRY_MS = 30000;
constexpr uint32_t DISPLAY_REFRESH_MS = 2500;
constexpr uint32_t DISPLAY_CLOCK_REFRESH_MS = 1000;
constexpr uint32_t DISPLAY_SLEEP_IDLE_MS = 300000;
constexpr uint32_t BUTTON_DEBOUNCE_MS = 45;
constexpr uint32_t BUTTON_SHORT_MIN_MS = 35;
constexpr uint32_t BUTTON_LONG_MS = 1200;
constexpr uint32_t BUTTON_STARTUP_SETTLE_MS = 2500;
constexpr uint32_t BATTERY_REFRESH_MS = 60000;
constexpr uint32_t BLE_SCAN_RETRY_MS = 15000;
constexpr uint32_t BLE_POLL_ANALOG_MS = 5000;
constexpr uint32_t BLE_POLL_ANALOG_IDLE_MS = 60000;
constexpr uint32_t BLE_POLL_WARNING_MS = 30000;
constexpr uint32_t BLE_POLL_CELLS_MS = 15000;
constexpr uint32_t BMS_STALE_MS = 120000;
constexpr uint32_t RUNTIME_SAVE_MS = 60000;

constexpr const char *BMS_BLE_SERVICE_UUID = "0000fff0-0000-1000-8000-00805f9b34fb";
constexpr const char *BMS_BLE_NOTIFY_UUID = "0000fff1-0000-1000-8000-00805f9b34fb";
constexpr const char *BMS_BLE_WRITE_UUID = "0000fff2-0000-1000-8000-00805f9b34fb";
constexpr const char *BMS_BLE_AUTH_UUID = "0000fffa-0000-1000-8000-00805f9b34fb";
constexpr const char *JBD_FF00_SERVICE_UUID = "0000ff00-0000-1000-8000-00805f9b34fb";
constexpr const char *JBD_FF01_NOTIFY_UUID = "0000ff01-0000-1000-8000-00805f9b34fb";
constexpr const char *JBD_FF02_WRITE_UUID = "0000ff02-0000-1000-8000-00805f9b34fb";
constexpr const char *JBD_FFE0_SERVICE_UUID = "0000ffe0-0000-1000-8000-00805f9b34fb";
constexpr const char *JBD_FFE1_DATA_UUID = "0000ffe1-0000-1000-8000-00805f9b34fb";

constexpr uint16_t DP_ANALOG_QUANTITY = 140;
constexpr uint16_t DP_WARNING_INFO = 141;
constexpr uint16_t DP_PRODUCT_INFO = 146;
constexpr uint8_t JBD_CMD_BASIC_INFO = 0x03;
constexpr uint8_t JBD_CMD_CELL_VOLTAGES = 0x04;

constexpr float DEFAULT_TYPICAL_MOW_AMPS = 35.0f;
constexpr float LOAD_FULL_SCALE_AMPS = 180.0f;
constexpr float DEFAULT_MOWER_ON_AMPS = 5.0f;
constexpr float DEFAULT_BLADES_ON_AMPS = 18.0f;

struct ThemeProfile {
  const char *id;
  const char *label;
  const char *family;
  const char *description;
  uint32_t bg;
  uint32_t panel;
  uint32_t panel2;
  uint32_t line;
  uint32_t text;
  uint32_t muted;
  uint32_t primary;
  uint32_t primaryDark;
  uint32_t accent;
  uint32_t warn;
  uint32_t bad;
  uint32_t buttonText;
};

struct UsageCategory {
  const char *id;
  const char *label;
  const char *defaultThemeId;
  const char *activeLabel;
  const char *workLabel;
  const char *activeMode;
  const char *workMode;
  const char *standbyMode;
  bool audioAssist;
};

enum class BmsParser : uint8_t {
  Watt,
  Jbd,
};

struct BmsProfile {
  const char *id;
  const char *label;
  const char *serviceUuid;
  const char *notifyUuid;
  const char *writeUuid;
  const char *authUuid;
  const char *authText;
  bool writeWithResponse;
  BmsParser parser;
};

constexpr BmsProfile BMS_PROFILES[] = {
    {"humsienk_watt", "Humsienk / Hoperf WATT BLE", BMS_BLE_SERVICE_UUID,
     BMS_BLE_NOTIFY_UUID, BMS_BLE_WRITE_UUID, BMS_BLE_AUTH_UUID, "HiLink", true, BmsParser::Watt},
    {"jbd_xiaoxiang_ff00", "JBD / Xiaoxiang BLE UART (FF00/FF01/FF02)",
     JBD_FF00_SERVICE_UUID, JBD_FF01_NOTIFY_UUID, JBD_FF02_WRITE_UUID, nullptr, nullptr, false,
     BmsParser::Jbd},
    {"jbd_xiaoxiang_ffe0", "JBD / Xiaoxiang BLE UART (FFE0/FFE1)",
     JBD_FFE0_SERVICE_UUID, JBD_FFE1_DATA_UUID, JBD_FFE1_DATA_UUID, nullptr, nullptr, false,
     BmsParser::Jbd},
};

constexpr ThemeProfile THEME_PROFILES[] = {
    {"chlorophyll_shift", "Chlorophyll Shift", "green",
     "Deep green utility look for mower and off-grid battery dashboards.",
     0x050605, 0x10170D, 0x17220F, 0x2A3524,
     0xF6FAF0, 0x9CAA92, 0xB7F23B, 0x4F7B13, 0x40E0D0, 0xFFD166, 0xFF4D4D, 0x071007},
    {"redline_charge", "Redline Charge", "red",
     "High-contrast red scheme for carts, utility haulers, and performance builds.",
     0x090606, 0x171010, 0x231313, 0x3A2424,
     0xFFF7F2, 0xC7A39C, 0xFF4D4D, 0x9F2020, 0xFFD166, 0xFFB703, 0x7DE2D1, 0x160404},
    {"violet_voltage", "Violet Voltage", "purple",
     "Purple and cyan palette for custom packs and show builds.",
     0x07060B, 0x12101B, 0x1F1730, 0x352B4B,
     0xFCF7FF, 0xB9A7CF, 0xC77DFF, 0x6D37A8, 0x64D2FF, 0xFFE066, 0xFF5C8A, 0x12051D},
    {"blue_fairway", "Blue Fairway", "blue",
     "Clean blue interface for golf carts, marine packs, and fleet equipment.",
     0x05080D, 0x0E1622, 0x13243A, 0x263A52,
     0xF2F8FF, 0x9EB2C7, 0x49B6FF, 0x1768AC, 0x7CFFCB, 0xF9C74F, 0xFF5A5F, 0x03111D},
    {"orange_ignition", "Orange Ignition", "orange",
     "Warm orange scheme for trailers, RVs, and shop power systems.",
     0x0A0704, 0x181109, 0x26180A, 0x3E2B17,
     0xFFF9F0, 0xC8AA84, 0xFF9F1C, 0xB85C00, 0x2EC4B6, 0xFFE066, 0xE71D36, 0x1B0900},
    {"parade_current", "Parade Current", "multicolored",
     "Loud, bright, flamboyant, and proud for installs that should not blend in.",
     0x07030C, 0x170C28, 0x241044, 0x5F2BFF,
     0xFFFFFF, 0xFFD6FF, 0xF72585, 0xB5179E, 0x00F5D4, 0xFEE440, 0xFF006E, 0x120016},
    {"pixel_fairway", "Pixel Fairway", "retro",
     "Retro display style for older carts, restored equipment, and DIY cabinets.",
     0x080A06, 0x15180D, 0x222510, 0x4D5830,
     0xF3F0C2, 0xB9B47B, 0xD6D93B, 0x6B7F1A, 0xF28C28, 0xF2D16B, 0xD1495B, 0x101204},
    {"modern_graphite", "Modern Graphite", "modern",
     "Quiet modern palette for power stations, RV panels, and clean dash installs.",
     0x07090B, 0x11151A, 0x1A2028, 0x303946,
     0xF4F7FA, 0xAAB3BE, 0xE5F0FF, 0x7D8EA3, 0x79FFE1, 0xFFD166, 0xFF5C7A, 0x05090D},
};

constexpr UsageCategory USAGE_CATEGORIES[] = {
    {"mower", "Mower", "chlorophyll_shift", "run", "mow load", "driving", "mowing", "standby", true},
    {"golf_cart", "Golf Cart", "blue_fairway", "drive", "climb load", "driving", "climbing", "parked", false},
    {"trailer_rv", "Trailer / RV", "orange_ignition", "load", "high draw", "supplying loads", "high draw", "standby", false},
    {"power_station", "Power Station", "modern_graphite", "output", "surge load", "supplying power", "surge load", "standby", false},
    {"marine_trolling", "Marine / Trolling", "blue_fairway", "run", "trolling load", "running", "trolling", "standby", false},
    {"utility_cart", "Utility Cart", "redline_charge", "drive", "work load", "driving", "hauling", "parked", false},
    {"off_grid", "Off-Grid / Solar", "pixel_fairway", "load", "inverter load", "supplying loads", "inverter load", "standby", false},
    {"custom", "Custom Large Pack", "parade_current", "activity", "high load", "active", "high load", "standby", false},
};

struct AppSettings {
  String hostname;
  String apPassword = "r48display";
  String otaPassword = "r48display";
  String wifiSsid;
  String wifiPassword;
  String bmsName;
  String bmsProtocol = "humsienk_watt";
  String mowerModel = "48V battery system";
  String usageCategory = "mower";
  String themeId = "chlorophyll_shift";
  bool dischargeCurrentNegative = true;
  bool displayEnabled = true;
  bool activityDetection = true;
  bool workDetection = true;
  float typicalMowAmps = DEFAULT_TYPICAL_MOW_AMPS;
  float mowerRunAmps = DEFAULT_MOWER_ON_AMPS;
  float bladesOnAmps = DEFAULT_BLADES_ON_AMPS;
  float nominalPackAh = 100.0f;
  String tempUnit = "F";
  String timezone = "PST8PDT,M3.2.0,M11.1.0";
  uint8_t brightness = 210;
  uint16_t displayRotation = 0;
  bool featureMic = false;
  float micRunThreshold = 900.0f;
};

struct BatterySample {
  float volts = 0.0f;
  float previousVolts = 0.0f;
  int16_t deltaMv = 0;
  uint8_t percent = 0;
  String label = "unknown";
  String inputStatus = "unknown";
  bool present = false;
  bool usbCdcConnected = false;
  uint32_t updatedMs = 0;
};

struct BmsBleData {
  bool enabled = true;
  bool initialized = false;
  bool scanning = false;
  bool found = false;
  bool connected = false;
  bool authenticated = false;
  bool notifyReady = false;
  String address;
  String name;
  String status = "not started";
  String lastError;
  int rssi = 0;
  uint32_t frames = 0;
  uint32_t bytes = 0;
  uint32_t errors = 0;
  uint32_t lastScanMs = 0;
  uint32_t lastConnectMs = 0;
  uint32_t lastRxMs = 0;
  uint32_t lastPollMs = 0;
  uint32_t lastAnalogPollMs = 0;
  uint32_t lastWarningPollMs = 0;
  uint32_t lastProductPollMs = 0;
  uint32_t lastCellPollMs = 0;
  uint32_t lastAnalogMs = 0;
  uint32_t lastWarningMs = 0;
  uint32_t lastProductMs = 0;
  String latestFrameHex;
  String warningHex;
  String software;
  String manufacturer;
  String serial;
  uint8_t cellCount = 0;
  uint8_t tempCount = 0;
  float cellVolts[32]{};
  float mosTempC = 0.0f;
  float pcbTempC = 0.0f;
  float tempsC[16]{};
  float currentA = 0.0f;
  float balanceCurrentA = 0.0f;
  float packVoltage = 0.0f;
  float remainingAh = 0.0f;
  float totalAh = 0.0f;
  float designAh = 0.0f;
  uint16_t cycleCount = 0;
  uint16_t soc = 0;
  float minCellV = 0.0f;
  float maxCellV = 0.0f;
  float deltaCellMv = 0.0f;
};

struct TouchPoint {
  uint16_t x = 0;
  uint16_t y = 0;
  uint8_t gesture = 0;
};

struct ButtonTracker {
  ButtonTracker() = default;
  ButtonTracker(uint8_t buttonPin, const char *buttonName, bool displayToggle)
      : pin(buttonPin), name(buttonName), togglesDisplay(displayToggle) {}

  uint8_t pin = 0;
  bool idleLow = false;
  bool togglesDisplay = false;
  bool startupSettled = false;
  bool rawPressed = false;
  bool stablePressed = false;
  uint32_t changedMs = 0;
  uint32_t initMs = 0;
  uint32_t pressedMs = 0;
  const char *name = "";
};

Preferences prefs;
WebServer server(80);
TwoWire touchWire(1);

Arduino_DataBus *displayBus = new Arduino_ESP32QSPI(
    PIN_LCD_CS, PIN_LCD_SCK, PIN_LCD_D0, PIN_LCD_D1, PIN_LCD_D2, PIN_LCD_D3);
Arduino_GFX *gfx = new Arduino_ST77916(
    displayBus, GFX_NOT_DEFINED, 0, true, DISPLAY_WIDTH, DISPLAY_HEIGHT);

AppSettings settings;
BatterySample screenBattery;
BmsBleData bms;

String internalI2cAddresses;
String touchI2cAddresses;
String apSsid;
bool displayReady = false;
bool touchReady = false;
bool provisioningActive = false;
bool mdnsReady = false;
bool displaySleeping = false;
bool displayManualOff = false;
bool ntpConfigured = false;
bool pendingStaStart = false;
bool pendingProvisioningStart = false;
uint8_t ioExpanderOutput = 0x07;
uint8_t displayPage = 0;
uint32_t lastWifiAttemptMs = 0;
uint32_t lastDisplayMs = 0;
uint32_t lastBatteryMs = 0;
uint32_t lastTouchProbeMs = 0;
uint32_t lastTouchMs = 0;
uint32_t lastDisplayActivityMs = 0;
uint64_t runtimeSeconds = 0;
uint64_t rideSessionSeconds = 0;
uint32_t runTimeRemainderMs = 0;
uint32_t lastRuntimeTickMs = 0;
uint32_t lastRuntimeSaveMs = 0;
bool previousCharging = false;
String lastButtonAction;
uint32_t lastButtonActionMs = 0;
ButtonTracker bootButton{static_cast<uint8_t>(PIN_BOOT_BUTTON), "BOOT", true};
ButtonTracker powerButton{static_cast<uint8_t>(PIN_BATTERY_KEY), "POWER", true};

String escapeHtml(const String &input);
void drawDisplay(bool fullRedraw = false);
void setupRoutes();
void saveSettings();
void saveRuntime();
void startStaOnly();
void startProvisioningAp();
void configureClock();

String chipSuffix() {
  char suffix[7];
  snprintf(suffix, sizeof(suffix), "%06X", static_cast<uint32_t>(ESP.getEfuseMac() & 0xFFFFFF));
  return String(suffix);
}

String defaultHostname() {
  String host = "r48display-";
  host += chipSuffix();
  host.toLowerCase();
  return host;
}

String sanitizeHostname(String value) {
  value.trim();
  value.toLowerCase();
  String out;
  out.reserve(min<size_t>(value.length(), 31));
  for (size_t i = 0; i < value.length() && out.length() < 31; ++i) {
    const char c = value[i];
    if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-') out += c;
  }
  while (out.startsWith("-")) out.remove(0, 1);
  while (out.endsWith("-")) out.remove(out.length() - 1);
  return out.length() ? out : defaultHostname();
}

uint16_t normalizeDisplayRotation(int value) {
  if (value >= 0 && value <= 3) value *= 90;
  switch (value) {
    case 90:
    case 180:
    case 270:
      return static_cast<uint16_t>(value);
    default:
      return 0;
  }
}

uint8_t displayRotationStep() {
  return static_cast<uint8_t>((settings.displayRotation / 90) % 4);
}

void applyDisplayRotation() {
  if (!gfx) return;
  gfx->setRotation(displayRotationStep());
}

String ipToString(const IPAddress &ip) {
  if (ip == INADDR_NONE) return "0.0.0.0";
  return ip.toString();
}

String millisAge(uint32_t eventMs) {
  if (eventMs == 0) return "never";
  const uint32_t age = millis() - eventMs;
  if (age < 1000) return String(age) + " ms ago";
  if (age < 60000) return String(age / 1000) + " s ago";
  if (age < 3600000) return String(age / 60000) + " min ago";
  return String(age / 3600000) + " h ago";
}

String formatDuration(uint64_t seconds) {
  const uint64_t hours = seconds / 3600ULL;
  const uint64_t mins = (seconds % 3600ULL) / 60ULL;
  if (hours > 0) return String(static_cast<unsigned long>(hours)) + "h " + String(static_cast<unsigned long>(mins)) + "m";
  return String(static_cast<unsigned long>(mins)) + "m";
}

String wifiSignalLabel(int32_t rssi) {
  if (rssi >= -55) return "excellent";
  if (rssi >= -67) return "good";
  if (rssi >= -75) return "fair";
  return "weak";
}

String currentTimeText(const char *fmt) {
  struct tm info;
  if (!getLocalTime(&info, 50)) return "--";
  char buffer[32];
  strftime(buffer, sizeof(buffer), fmt, &info);
  return String(buffer);
}

String formatHours(float hours) {
  if (hours <= 0.0f || !isfinite(hours)) return "--";
  if (hours < 1.0f) return String(hours * 60.0f, 0) + " min";
  return String(hours, 1) + " h";
}

float displayTempC(float celsius) {
  return settings.tempUnit == "F" ? celsius * 9.0f / 5.0f + 32.0f : celsius;
}

String formatTemp(float celsius, uint8_t decimals = 0) {
  if (!isfinite(celsius) || fabsf(celsius) < 0.01f) return "--";
  return String(displayTempC(celsius), static_cast<unsigned int>(decimals)) + settings.tempUnit;
}

const BmsProfile &activeProfile() {
  for (const auto &profile : BMS_PROFILES) {
    if (settings.bmsProtocol == profile.id) return profile;
  }
  return BMS_PROFILES[0];
}

const ThemeProfile &activeTheme() {
  for (const auto &theme : THEME_PROFILES) {
    if (settings.themeId == theme.id) return theme;
  }
  return THEME_PROFILES[0];
}

bool validThemeId(const String &id) {
  for (const auto &theme : THEME_PROFILES) {
    if (id == theme.id) return true;
  }
  return false;
}

const UsageCategory &activeUsage() {
  for (const auto &usage : USAGE_CATEGORIES) {
    if (settings.usageCategory == usage.id) return usage;
  }
  return USAGE_CATEGORIES[0];
}

bool validUsageCategory(const String &id) {
  for (const auto &usage : USAGE_CATEGORIES) {
    if (id == usage.id) return true;
  }
  return false;
}

String cssColor(uint32_t value) {
  char buffer[8];
  snprintf(buffer, sizeof(buffer), "#%06lX", static_cast<unsigned long>(value & 0xFFFFFFUL));
  return String(buffer);
}

bool deviceAdvertisesProfile(const NimBLEAdvertisedDevice *device, const BmsProfile &profile) {
  return device && device->isAdvertisingService(NimBLEUUID(profile.serviceUuid));
}

const BmsProfile *firstCompatibleProfile(const NimBLEAdvertisedDevice *device) {
  for (const auto &profile : BMS_PROFILES) {
    if (deviceAdvertisesProfile(device, profile)) return &profile;
  }
  return nullptr;
}

float capacityScale() {
  if (settings.nominalPackAh > 0.1f && bms.designAh > 0.1f) {
    return settings.nominalPackAh / bms.designAh;
  }
  return 1.0f;
}

float effectiveRemainingAh() {
  return bms.remainingAh * capacityScale();
}

float effectiveTotalAh() {
  return bms.totalAh * capacityScale();
}

float effectiveDesignAh() {
  return settings.nominalPackAh > 0.1f ? settings.nominalPackAh : bms.designAh;
}

float signedDischargeCurrentA() {
  return settings.dischargeCurrentNegative ? -bms.currentA : bms.currentA;
}

float dischargeCurrentA() {
  return max(0.0f, signedDischargeCurrentA());
}

float chargeCurrentA() {
  return max(0.0f, -signedDischargeCurrentA());
}

float loadPercent() {
  return constrain((dischargeCurrentA() / LOAD_FULL_SCALE_AMPS) * 100.0f, 0.0f, 100.0f);
}

float dischargeWatts() {
  return bms.packVoltage * dischargeCurrentA();
}

float chargeWatts() {
  return bms.packVoltage * chargeCurrentA();
}

float runtimeEstimateHours() {
  const float amps = dischargeCurrentA() > 1.0f ? dischargeCurrentA() : settings.typicalMowAmps;
  const float remainingAh = effectiveRemainingAh();
  if (remainingAh <= 0.1f || amps <= 0.1f) return 0.0f;
  return remainingAh / amps;
}

float chargeEstimateHours() {
  const float missingAh = max(0.0f, effectiveTotalAh() - effectiveRemainingAh());
  if (missingAh <= 0.1f || chargeCurrentA() <= 0.2f) return 0.0f;
  return missingAh / chargeCurrentA();
}

float bmsHealthPercent() {
  float health = 0.0f;
  const float designAh = effectiveDesignAh();
  const float totalAh = effectiveTotalAh();
  if (designAh > 0.1f && totalAh > 0.1f) {
    health = (totalAh / designAh) * 100.0f;
  } else if (totalAh > 0.1f) {
    health = 100.0f;
  }
  if (bms.deltaCellMv > 50.0f) {
    health -= min(10.0f, (bms.deltaCellMv - 50.0f) / 10.0f);
  }
  return constrain(health, 0.0f, 100.0f);
}

bool packCharging() {
  return chargeCurrentA() >= 0.2f;
}

bool activityDetected() {
  const float amps = dischargeCurrentA();
  if (!settings.activityDetection) return amps >= 0.2f;
  return amps >= settings.mowerRunAmps;
}

bool workDetected() {
  if (!settings.workDetection) return false;
  const UsageCategory &usage = activeUsage();
  const bool audioAssist = usage.audioAssist && settings.featureMic && activityDetected() && R48Mic::toneDetected();
  return dischargeCurrentA() >= settings.bladesOnAmps || audioAssist;
}

bool mowerRunning() {
  return activityDetected();
}

bool bladesLikelyOn() {
  return workDetected();
}

String mowerModeLabel() {
  if (packCharging()) return "charging";
  const UsageCategory &usage = activeUsage();
  if (!settings.activityDetection) return dischargeCurrentA() >= 0.2f ? "discharging" : usage.standbyMode;
  if (workDetected()) return usage.workMode;
  if (activityDetected()) return usage.activeMode;
  return usage.standbyMode;
}

const char *displayPageName(uint8_t page) {
  switch (page % R48DisplayUi::PAGE_COUNT) {
    case 0: return "dashboard";
    case 1: return "power";
    case 2: return "clock";
    default: return "status";
  }
}

String bmsLinkLabel() {
  if (bms.connected && bms.lastAnalogMs > 0 && millis() - bms.lastAnalogMs > BMS_STALE_MS) return "stale";
  if (bms.connected && bms.authenticated && bms.lastAnalogMs > 0) return "online";
  if (bms.connected) return "connected";
  if (bms.scanning) return "scanning";
  return "searching";
}

class BleBmsClient;
BleBmsClient *bleClientForNotify = nullptr;

class BleBmsClient {
 public:
  void begin() {
    if (bms.initialized) return;
    NimBLEDevice::init(settings.hostname.c_str());
    NimBLEDevice::setPower(3);
    NimBLEScan *scan = NimBLEDevice::getScan();
    scan->setActiveScan(true);
    scan->setInterval(160);
    scan->setWindow(80);
    scan->setMaxResults(16);
    bleClientForNotify = this;
    bms.initialized = true;
    bms.status = "BLE ready";
  }

  void loop() {
    drainNotifyQueue();
    if (!bms.enabled) return;
    if (!bms.initialized) begin();
    const uint32_t now = millis();
    if (client_ && bms.connected && !client_->isConnected()) {
      resetClient(false);
      bms.status = "disconnected, rescanning";
      bms.lastError = "BLE connection dropped";
    }
    if (!bms.connected) {
      if (now - bms.lastScanMs >= BLE_SCAN_RETRY_MS || bms.lastScanMs == 0) scanAndConnect();
      return;
    }
    if (now - lastRssiMs_ > 5000) {
      lastRssiMs_ = now;
      bms.rssi = client_->getRssi();
    }
    if (now - bms.lastPollMs < 900) return;
    if (bms.lastAnalogMs > 0 && now - bms.lastAnalogMs > BMS_STALE_MS && bms.status != "stale data") {
      bms.status = "stale data";
      bms.lastError = "BMS data stale";
    }
    const uint32_t analogInterval = displaySleeping ? BLE_POLL_ANALOG_IDLE_MS : BLE_POLL_ANALOG_MS;
    if (bms.lastAnalogMs == 0 || now - bms.lastAnalogPollMs >= analogInterval || bms.lastAnalogPollMs == 0) {
      requestAnalog();
      bms.lastAnalogPollMs = now;
      return;
    }
    if (activeProfile().parser == BmsParser::Jbd &&
        (bms.cellCount == 0 || now - bms.lastCellPollMs >= BLE_POLL_CELLS_MS || bms.lastCellPollMs == 0)) {
      requestCells();
      bms.lastCellPollMs = now;
      return;
    }
    if (activeProfile().parser == BmsParser::Watt && (bms.lastProductMs == 0 || now - bms.lastProductPollMs > 60000)) {
      requestProduct();
      bms.lastProductPollMs = now;
      return;
    }
    if (activeProfile().parser == BmsParser::Watt &&
        (now - bms.lastWarningPollMs >= BLE_POLL_WARNING_MS || bms.lastWarningPollMs == 0)) {
      requestWarning();
      bms.lastWarningPollMs = now;
    }
  }

  void reconnect() {
    resetClient(true);
    bms.lastScanMs = 0;
    bms.status = "manual reconnect";
  }

  bool readNow() {
    if (!ready()) {
      bms.lastError = "BMS not connected";
      return false;
    }
    bool ok = requestAnalog();
    ok = requestCells() && ok;
    ok = requestProduct() && ok;
    ok = requestWarning() && ok;
    return ok;
  }

  void onNotify(uint8_t *data, size_t length) {
    if (!data || length == 0) return;
    portENTER_CRITICAL(&notifyMux_);
    for (size_t i = 0; i < length; ++i) {
      const size_t next = (notifyHead_ + 1) % NOTIFY_QUEUE_SIZE;
      if (next == notifyTail_) {
        notifyDroppedBytes_++;
        break;
      }
      notifyQueue_[notifyHead_] = data[i];
      notifyHead_ = next;
      notifyQueuedBytes_++;
    }
    portEXIT_CRITICAL(&notifyMux_);
  }

 private:
  static constexpr size_t NOTIFY_QUEUE_SIZE = 1024;

  bool ready() const {
    return client_ && client_->isConnected() && writeChar_ && notifyChar_ && bms.authenticated;
  }

  void resetClient(bool disconnect) {
    if (client_) {
      if (disconnect && client_->isConnected()) client_->disconnect();
      NimBLEDevice::deleteClient(client_);
    }
    client_ = nullptr;
    notifyChar_ = nullptr;
    writeChar_ = nullptr;
    authChar_ = nullptr;
    rxLen_ = 0;
    notifyHead_ = 0;
    notifyTail_ = 0;
    notifyQueuedBytes_ = 0;
    notifyDroppedBytes_ = 0;
    bms.connected = false;
    bms.authenticated = false;
    bms.notifyReady = false;
  }

  bool deviceMatches(const NimBLEAdvertisedDevice *device, const BmsProfile &profile) {
    if (!device) return false;
    const String target = settings.bmsName;
    const String name(device->getName().c_str());
    const String address(device->getAddress().toString().c_str());
    if (target.length() > 0) {
      return name == target || address.equalsIgnoreCase(target);
    }
    return deviceAdvertisesProfile(device, profile);
  }

  void scanAndConnect() {
    const BmsProfile &profile = activeProfile();
    bms.scanning = true;
    bms.status = settings.bmsName.length() ? "scanning for " + settings.bmsName : "scanning for compatible BMS";
    bms.lastScanMs = millis();
    NimBLEScan *scan = NimBLEDevice::getScan();
    scan->clearResults();
    NimBLEScanResults results = scan->getResults(3000, false);
    const NimBLEAdvertisedDevice *target = nullptr;
    for (int i = 0; i < results.getCount(); ++i) {
      const NimBLEAdvertisedDevice *device = results.getDevice(i);
      if (deviceMatches(device, profile)) {
        target = device;
        break;
      }
    }
    if (target) {
      bms.found = true;
      bms.address = String(target->getAddress().toString().c_str());
      bms.name = String(target->getName().c_str());
      bms.rssi = target->getRSSI();
      connectToTarget(target, profile);
    } else {
      bms.status = "compatible BMS not found";
      bms.lastError = settings.bmsName.length() ? "target not found in scan" : "no advertised compatible service";
    }
    scan->clearResults();
    bms.scanning = false;
  }

  void connectToTarget(const NimBLEAdvertisedDevice *device, const BmsProfile &profile) {
    resetClient(false);
    bms.status = "connecting BLE";
    client_ = NimBLEDevice::createClient();
    if (!client_) {
      bms.errors++;
      bms.lastError = "cannot allocate BLE client";
      return;
    }
    client_->setConnectTimeout(8000);
    client_->setConnectionParams(24, 48, 0, 500);
    if (!client_->connect(device)) {
      bms.errors++;
      bms.lastError = "BLE connect failed";
      resetClient(false);
      return;
    }
    bms.connected = true;
    bms.lastConnectMs = millis();
    bms.rssi = client_->getRssi();
    NimBLERemoteService *service = client_->getService(profile.serviceUuid);
    if (!service) {
      bms.errors++;
      bms.lastError = "BMS service not found";
      resetClient(true);
      return;
    }
    notifyChar_ = service->getCharacteristic(profile.notifyUuid);
    writeChar_ = service->getCharacteristic(profile.writeUuid);
    authChar_ = profile.authUuid ? service->getCharacteristic(profile.authUuid) : nullptr;
    if (!notifyChar_ || !writeChar_ || (profile.authUuid && !authChar_)) {
      bms.errors++;
      bms.lastError = "BMS characteristics missing";
      resetClient(true);
      return;
    }
    if (!notifyChar_->subscribe(true, notifyCallback, true)) {
      bms.errors++;
      bms.lastError = "BLE notify subscribe failed";
      resetClient(true);
      return;
    }
    bms.notifyReady = true;
    if (profile.authText && authChar_) {
      const uint8_t *auth = reinterpret_cast<const uint8_t *>(profile.authText);
      if (!authChar_->writeValue(auth, strlen(profile.authText), true)) {
        bms.errors++;
        bms.lastError = "BLE auth failed";
        resetClient(true);
        return;
      }
    }
    bms.authenticated = true;
    bms.lastError = "";
    bms.status = "connected, authenticated";
    requestProduct();
    requestAnalog();
    bms.lastProductPollMs = millis();
  }

  void drainNotifyQueue() {
    uint8_t local[128];
    while (true) {
      size_t count = 0;
      uint32_t dropped = 0;
      portENTER_CRITICAL(&notifyMux_);
      while (notifyTail_ != notifyHead_ && count < sizeof(local)) {
        local[count++] = notifyQueue_[notifyTail_];
        notifyTail_ = (notifyTail_ + 1) % NOTIFY_QUEUE_SIZE;
      }
      if (notifyQueuedBytes_ >= count) notifyQueuedBytes_ -= count;
      dropped = notifyDroppedBytes_;
      notifyDroppedBytes_ = 0;
      portEXIT_CRITICAL(&notifyMux_);

      if (dropped > 0) {
        bms.errors++;
        bms.lastError = "BLE notify queue overflow";
      }
      if (count == 0) return;
      appendRxBytes(local, count);
    }
  }

  void appendRxBytes(const uint8_t *data, size_t length) {
    bms.bytes += length;
    bms.lastRxMs = millis();
    if (rxLen_ + length > sizeof(rxBuffer_)) {
      rxLen_ = 0;
      bms.errors++;
      bms.lastError = "BLE frame buffer overflow";
      return;
    }
    memcpy(rxBuffer_ + rxLen_, data, length);
    rxLen_ += length;
    consumeRxBuffer();
  }

  bool requestAnalog() {
    const BmsProfile &profile = activeProfile();
    if (profile.parser == BmsParser::Jbd) return requestJbdRead(JBD_CMD_BASIC_INFO);
    return requestWattRead(DP_ANALOG_QUANTITY);
  }

  bool requestCells() {
    const BmsProfile &profile = activeProfile();
    if (profile.parser == BmsParser::Jbd) return requestJbdRead(JBD_CMD_CELL_VOLTAGES);
    return true;
  }

  bool requestProduct() {
    const BmsProfile &profile = activeProfile();
    if (profile.parser != BmsParser::Watt) return true;
    return requestWattRead(DP_PRODUCT_INFO);
  }

  bool requestWarning() {
    const BmsProfile &profile = activeProfile();
    if (profile.parser != BmsParser::Watt) return true;
    return requestWattRead(DP_WARNING_INFO);
  }

  bool requestWattRead(uint16_t address) {
    if (!ready()) return false;
    uint8_t frame[11]{0x7E, 0x00, 0x01, 0x03, static_cast<uint8_t>(address >> 8),
                      static_cast<uint8_t>(address & 0xFF), 0x00, 0x00, 0x00, 0x00, 0x0D};
    const uint16_t crc = crc16Modbus(frame, 8);
    frame[8] = static_cast<uint8_t>(crc >> 8);
    frame[9] = static_cast<uint8_t>(crc & 0xFF);
    const bool ok = writeChar_->writeValue(frame, sizeof(frame), activeProfile().writeWithResponse);
    bms.lastPollMs = millis();
    if (!ok) {
      bms.errors++;
      bms.lastError = "BLE write failed";
    }
    return ok;
  }

  bool requestJbdRead(uint8_t command) {
    if (!ready()) return false;
    uint8_t frame[7]{0xDD, 0xA5, command, 0x00, 0x00, 0x00, 0x77};
    const uint16_t checksum = jbdChecksum(frame + 2, 2);
    frame[4] = static_cast<uint8_t>(checksum >> 8);
    frame[5] = static_cast<uint8_t>(checksum & 0xFF);
    const bool ok = writeChar_->writeValue(frame, sizeof(frame), activeProfile().writeWithResponse);
    bms.lastPollMs = millis();
    if (!ok) {
      bms.errors++;
      bms.lastError = "BLE write failed";
    }
    return ok;
  }

  void consumeRxBuffer() {
    while (rxLen_ > 0) {
      const uint8_t expectedHead = activeProfile().parser == BmsParser::Jbd ? 0xDD : 0x7E;
      if (rxBuffer_[0] != expectedHead) {
        memmove(rxBuffer_, rxBuffer_ + 1, rxLen_ - 1);
        rxLen_--;
        bms.errors++;
        continue;
      }
      const size_t expected = activeProfile().parser == BmsParser::Jbd ? expectedJbdLength() : expectedWattLength();
      if (expected == 0) return;
      if (expected > sizeof(rxBuffer_)) {
        rxLen_ = 0;
        bms.errors++;
        bms.lastError = "BLE frame too long";
        return;
      }
      if (rxLen_ < expected) return;
      parseFrame(rxBuffer_, expected);
      memmove(rxBuffer_, rxBuffer_ + expected, rxLen_ - expected);
      rxLen_ -= expected;
    }
  }

  size_t expectedWattLength() const {
    if (rxLen_ < 8) return 0;
    const uint16_t dataLen = readU16(rxBuffer_ + 6);
    return static_cast<size_t>(dataLen) + 11;
  }

  size_t expectedJbdLength() const {
    if (rxLen_ < 4) return 0;
    const uint8_t dataLen = rxBuffer_[3];
    return static_cast<size_t>(dataLen) + 7;
  }

  void parseFrame(const uint8_t *frame, size_t len) {
    if (activeProfile().parser == BmsParser::Jbd) {
      parseJbdFrame(frame, len);
      return;
    }
    parseWattFrame(frame, len);
  }

  void parseWattFrame(const uint8_t *frame, size_t len) {
    const uint16_t dataLen = readU16(frame + 6);
    if (len < 11 || frame[len - 1] != 0x0D || static_cast<size_t>(dataLen) + 11 != len) {
      bms.errors++;
      bms.lastError = "invalid BLE frame";
      return;
    }
    const uint16_t gotCrc = readU16(frame + len - 3);
    const uint16_t wantCrc = crc16Modbus(frame, len - 3);
    if (gotCrc != wantCrc) {
      bms.errors++;
      bms.lastError = "BLE CRC mismatch";
      return;
    }
    bms.lastError = "";
    bms.frames++;
    bms.latestFrameHex = bytesToHex(frame, len, 96);
    bms.status = "data received";
    const uint16_t start = readU16(frame + 4);
    const uint8_t *data = frame + 8;
    if (start == DP_PRODUCT_INFO) parseProduct(data, dataLen);
    else if (start == DP_ANALOG_QUANTITY) parseAnalog(data, dataLen);
    else if (start == DP_WARNING_INFO) {
      bms.warningHex = bytesToHex(data, dataLen, 96);
      bms.lastWarningMs = millis();
    }
  }

  void parseJbdFrame(const uint8_t *frame, size_t len) {
    if (len < 7 || frame[0] != 0xDD || frame[len - 1] != 0x77) {
      bms.errors++;
      bms.lastError = "invalid JBD frame";
      return;
    }
    const uint8_t command = frame[1];
    const uint8_t status = frame[2];
    const uint8_t dataLen = frame[3];
    if (static_cast<size_t>(dataLen) + 7 != len) {
      bms.errors++;
      bms.lastError = "invalid JBD length";
      return;
    }
    const uint16_t gotChecksum = readU16(frame + 4 + dataLen);
    const uint16_t wantChecksum = jbdChecksum(frame + 2, static_cast<size_t>(dataLen) + 2);
    if (gotChecksum != wantChecksum && status == 0) {
      bms.errors++;
      bms.lastError = "JBD checksum mismatch";
      return;
    }
    if (status != 0) {
      bms.errors++;
      bms.lastError = "JBD command rejected";
      return;
    }

    bms.lastError = "";
    bms.frames++;
    bms.latestFrameHex = bytesToHex(frame, len, 96);
    bms.status = "data received";
    const uint8_t *data = frame + 4;
    if (command == JBD_CMD_BASIC_INFO) parseJbdBasic(data, dataLen);
    else if (command == JBD_CMD_CELL_VOLTAGES) parseJbdCells(data, dataLen);
  }

  void parseProduct(const uint8_t *data, size_t len) {
    if (len < 60) return;
    bms.software = paddedAscii(data, 0, 20);
    bms.manufacturer = paddedAscii(data, 20, 20);
    bms.serial = paddedAscii(data, 40, 20);
    bms.lastProductMs = millis();
  }

  void parseAnalog(const uint8_t *data, size_t len) {
    size_t pos = 0;
    if (len < 1) return;
    const uint8_t cells = data[pos++];
    if (cells > 32 || pos + cells * 2 + 1 > len) return;
    bms.cellCount = cells;
    bms.minCellV = 0.0f;
    bms.maxCellV = 0.0f;
    for (uint8_t i = 0; i < cells; ++i) {
      const float volts = readU16(data + pos) / 1000.0f;
      pos += 2;
      bms.cellVolts[i] = volts;
      if (i == 0 || volts < bms.minCellV) bms.minCellV = volts;
      if (i == 0 || volts > bms.maxCellV) bms.maxCellV = volts;
    }
    bms.deltaCellMv = (bms.maxCellV - bms.minCellV) * 1000.0f;
    if (pos >= len) return;
    const uint8_t temps = data[pos++];
    if (temps > 18 || pos + temps * 2 > len) return;
    bms.tempCount = temps;
    bms.mosTempC = temps >= 1 ? rawTempC(readU16(data + pos)) : 0.0f;
    if (temps >= 1) pos += 2;
    bms.pcbTempC = temps >= 2 ? rawTempC(readU16(data + pos)) : 0.0f;
    if (temps >= 2) pos += 2;
    const uint8_t cellTemps = temps > 2 ? min<uint8_t>(temps - 2, 16) : 0;
    for (uint8_t i = 0; i < cellTemps; ++i) {
      bms.tempsC[i] = rawTempC(readU16(data + pos));
      pos += 2;
    }
    if (pos + 14 > len) return;
    bms.currentA = parseWattCurrent(data + pos);
    pos += 2;
    bms.packVoltage = readU16(data + pos) / 100.0f;
    pos += 2;
    bms.remainingAh = readU16(data + pos) / 10.0f;
    pos += 2;
    bms.totalAh = readU16(data + pos) / 10.0f;
    pos += 2;
    bms.cycleCount = readU16(data + pos);
    pos += 2;
    bms.designAh = readU16(data + pos) / 10.0f;
    pos += 2;
    bms.soc = readU16(data + pos);
    pos += 2;
    if (pos + 18 <= len) {
      pos += 14;
      bms.balanceCurrentA = parseWattCurrent(data + pos);
    }
    bms.lastAnalogMs = millis();
  }

  void parseJbdBasic(const uint8_t *data, size_t len) {
    if (len < 23) return;
    bms.manufacturer = "JBD / Xiaoxiang";
    bms.software = "JBD v";
    bms.software += String(data[18]);
    bms.packVoltage = readU16(data) / 100.0f;
    bms.currentA = static_cast<int16_t>(readU16(data + 2)) / 100.0f;
    bms.remainingAh = readU16(data + 4) / 100.0f;
    bms.totalAh = readU16(data + 6) / 100.0f;
    bms.designAh = bms.totalAh;
    bms.cycleCount = readU16(data + 8);
    bms.warningHex = bytesToHex(data + 16, 2, 2);
    bms.soc = data[19];
    bms.cellCount = min<uint8_t>(data[21], 32);
    const uint8_t temps = min<uint8_t>(data[22], 16);
    bms.tempCount = temps;
    bms.pcbTempC = temps >= 1 && len >= 25 ? jbdTempC(readU16(data + 23)) : 0.0f;
    bms.mosTempC = temps >= 2 && len >= 27 ? jbdTempC(readU16(data + 25)) : 0.0f;
    size_t pos = 23;
    for (uint8_t i = 0; i < temps && pos + 2 <= len; ++i) {
      bms.tempsC[i] = jbdTempC(readU16(data + pos));
      pos += 2;
    }
    bms.lastAnalogMs = millis();
  }

  void parseJbdCells(const uint8_t *data, size_t len) {
    const uint8_t cells = min<uint8_t>(len / 2, 32);
    if (cells == 0) return;
    bms.cellCount = cells;
    bms.minCellV = 0.0f;
    bms.maxCellV = 0.0f;
    for (uint8_t i = 0; i < cells; ++i) {
      const float volts = readU16(data + i * 2) / 1000.0f;
      bms.cellVolts[i] = volts;
      if (i == 0 || volts < bms.minCellV) bms.minCellV = volts;
      if (i == 0 || volts > bms.maxCellV) bms.maxCellV = volts;
    }
    bms.deltaCellMv = (bms.maxCellV - bms.minCellV) * 1000.0f;
    bms.lastAnalogMs = millis();
  }

  static uint16_t crc16Modbus(const uint8_t *data, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; ++i) {
      crc ^= data[i];
      for (uint8_t bit = 0; bit < 8; ++bit) {
        crc = (crc & 1) ? static_cast<uint16_t>((crc >> 1) ^ 0xA001) : static_cast<uint16_t>(crc >> 1);
      }
    }
    return crc;
  }

  static uint16_t jbdChecksum(const uint8_t *data, size_t len) {
    uint16_t sum = 0;
    for (size_t i = 0; i < len; ++i) sum += data[i];
    return static_cast<uint16_t>(0x10000UL - sum);
  }

  static uint16_t readU16(const uint8_t *data) {
    return (static_cast<uint16_t>(data[0]) << 8) | data[1];
  }

  static float rawTempC(uint16_t raw) {
    return (static_cast<int32_t>(raw) - 2730) / 10.0f;
  }

  static float jbdTempC(uint16_t raw) {
    return (static_cast<int32_t>(raw) - 2731) / 10.0f;
  }

  static float parseWattCurrent(const uint8_t *data) {
    const uint8_t high = data[0];
    const bool negative = (high & 0x80) != 0;
    const bool decimal = (high & 0x40) != 0;
    const uint16_t magnitude = (static_cast<uint16_t>(high & 0x3F) << 8) | data[1];
    const float value = decimal ? magnitude / 10.0f : static_cast<float>(magnitude);
    return negative ? -value : value;
  }

  static String paddedAscii(const uint8_t *data, size_t offset, size_t len) {
    String out;
    out.reserve(len);
    for (size_t i = 0; i < len; ++i) {
      const uint8_t c = data[offset + i];
      out += (c >= 32 && c <= 126) ? static_cast<char>(c) : ' ';
    }
    out.trim();
    return out;
  }

  static String bytesToHex(const uint8_t *data, size_t len, size_t maxLen) {
    static const char HEX_DIGITS[] = "0123456789ABCDEF";
    const size_t useLen = maxLen > 0 && len > maxLen ? maxLen : len;
    String out;
    out.reserve(useLen * 3 + 4);
    for (size_t i = 0; i < useLen; ++i) {
      if (i) out += ' ';
      out += HEX_DIGITS[data[i] >> 4];
      out += HEX_DIGITS[data[i] & 0x0F];
    }
    if (useLen < len) out += F(" ...");
    return out;
  }

  static void notifyCallback(NimBLERemoteCharacteristic *, uint8_t *data, size_t length, bool) {
    if (bleClientForNotify) bleClientForNotify->onNotify(data, length);
  }

  NimBLEClient *client_ = nullptr;
  NimBLERemoteCharacteristic *notifyChar_ = nullptr;
  NimBLERemoteCharacteristic *writeChar_ = nullptr;
  NimBLERemoteCharacteristic *authChar_ = nullptr;
  uint8_t rxBuffer_[384]{};
  size_t rxLen_ = 0;
  uint32_t lastRssiMs_ = 0;
  portMUX_TYPE notifyMux_ = portMUX_INITIALIZER_UNLOCKED;
  uint8_t notifyQueue_[NOTIFY_QUEUE_SIZE]{};
  size_t notifyHead_ = 0;
  size_t notifyTail_ = 0;
  volatile uint32_t notifyQueuedBytes_ = 0;
  volatile uint32_t notifyDroppedBytes_ = 0;
};

BleBmsClient bleBms;

void loadSettings() {
  prefs.begin("r48disp", false);
  settings.hostname = prefs.getString("host", defaultHostname());
  settings.apPassword = prefs.getString("apPass", settings.apPassword);
  settings.otaPassword = prefs.getString("otaPass", settings.otaPassword);
  settings.wifiSsid = prefs.getString("ssid", "");
  settings.wifiPassword = prefs.getString("wifiPass", "");
  settings.bmsName = prefs.getString("bmsName", "");
  settings.bmsProtocol = prefs.getString("bmsProto", settings.bmsProtocol);
  settings.mowerModel = prefs.getString("model", settings.mowerModel);
  settings.usageCategory = prefs.getString("usage", prefs.getString("vehicle", settings.usageCategory));
  settings.themeId = prefs.getString("theme", settings.themeId);
  settings.dischargeCurrentNegative = prefs.getBool("curNeg", true);
  settings.displayEnabled = prefs.getBool("display", true);
  settings.activityDetection = prefs.getBool("activity", settings.activityDetection);
  settings.workDetection = prefs.getBool("workDetect", settings.workDetection);
  settings.typicalMowAmps = prefs.getFloat("mowAmps", DEFAULT_TYPICAL_MOW_AMPS);
  settings.mowerRunAmps = prefs.getFloat("runAmps", DEFAULT_MOWER_ON_AMPS);
  settings.bladesOnAmps = prefs.getFloat("bladeAmps", DEFAULT_BLADES_ON_AMPS);
  settings.nominalPackAh = prefs.getFloat("packAh", 100.0f);
  settings.tempUnit = prefs.getString("tempUnit", settings.tempUnit);
  settings.timezone = prefs.getString("tz", settings.timezone);
  settings.brightness = prefs.getUChar("bright", 210);
  settings.displayRotation = normalizeDisplayRotation(prefs.getUShort("rotation", 0));
  settings.featureMic = prefs.getBool("mic", settings.featureMic);
  settings.micRunThreshold = prefs.getFloat("micTh", settings.micRunThreshold);
  runtimeSeconds = prefs.getULong64("runtime", 0);
  prefs.end();

  settings.hostname = sanitizeHostname(settings.hostname);
  if (settings.apPassword.length() < 8) settings.apPassword = "r48display";
  if (settings.otaPassword.length() < 8) settings.otaPassword = "r48display";
  if (!validUsageCategory(settings.usageCategory)) settings.usageCategory = USAGE_CATEGORIES[0].id;
  if (!validThemeId(settings.themeId)) settings.themeId = activeUsage().defaultThemeId;
  settings.typicalMowAmps = constrain(settings.typicalMowAmps, 5.0f, 160.0f);
  settings.mowerRunAmps = constrain(settings.mowerRunAmps, 1.0f, 80.0f);
  settings.bladesOnAmps = constrain(settings.bladesOnAmps, settings.mowerRunAmps, 200.0f);
  settings.nominalPackAh = constrain(settings.nominalPackAh, 1.0f, 400.0f);
  settings.tempUnit.toUpperCase();
  if (settings.tempUnit != "C") settings.tempUnit = "F";
  if (settings.timezone.isEmpty()) settings.timezone = "PST8PDT,M3.2.0,M11.1.0";
  settings.brightness = constrain(settings.brightness, static_cast<uint8_t>(20), static_cast<uint8_t>(255));
  settings.displayRotation = normalizeDisplayRotation(settings.displayRotation);
  settings.micRunThreshold = constrain(settings.micRunThreshold, 100.0f, 12000.0f);
  bool knownProtocol = false;
  for (const auto &profile : BMS_PROFILES) {
    if (settings.bmsProtocol == profile.id) knownProtocol = true;
  }
  if (!knownProtocol) settings.bmsProtocol = BMS_PROFILES[0].id;
}

void saveSettings() {
  prefs.begin("r48disp", false);
  prefs.putString("host", settings.hostname);
  prefs.putString("apPass", settings.apPassword);
  prefs.putString("otaPass", settings.otaPassword);
  prefs.putString("ssid", settings.wifiSsid);
  prefs.putString("wifiPass", settings.wifiPassword);
  prefs.putString("bmsName", settings.bmsName);
  prefs.putString("bmsProto", settings.bmsProtocol);
  prefs.putString("model", settings.mowerModel);
  prefs.putString("usage", settings.usageCategory);
  prefs.putString("theme", settings.themeId);
  prefs.putBool("curNeg", settings.dischargeCurrentNegative);
  prefs.putBool("display", settings.displayEnabled);
  prefs.putBool("activity", settings.activityDetection);
  prefs.putBool("workDetect", settings.workDetection);
  prefs.putFloat("mowAmps", settings.typicalMowAmps);
  prefs.putFloat("runAmps", settings.mowerRunAmps);
  prefs.putFloat("bladeAmps", settings.bladesOnAmps);
  prefs.putFloat("packAh", settings.nominalPackAh);
  prefs.putString("tempUnit", settings.tempUnit);
  prefs.putString("tz", settings.timezone);
  prefs.putUChar("bright", settings.brightness);
  prefs.putUShort("rotation", settings.displayRotation);
  prefs.putBool("mic", settings.featureMic);
  prefs.putFloat("micTh", settings.micRunThreshold);
  prefs.end();
}

void saveRuntime() {
  prefs.begin("r48disp", false);
  prefs.putULong64("runtime", runtimeSeconds);
  prefs.end();
  lastRuntimeSaveMs = millis();
}

bool tcaWrite(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(TCA9554_ADDR);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

void tcaSetOutput(uint8_t value) {
  ioExpanderOutput = value;
  tcaWrite(0x01, ioExpanderOutput);
}

void initIoExpander() {
  tcaWrite(0x03, 0xF0);
  tcaWrite(0x02, 0x00);
  tcaSetOutput(0x0C);
  delay(30);
  tcaSetOutput(0x0F);
  delay(150);
}

bool i2cDevicePresent(TwoWire &bus, uint8_t address) {
  bus.beginTransmission(address);
  return bus.endTransmission() == 0;
}

String i2cAddressCsv(TwoWire &bus) {
  String out;
  for (uint8_t address = 1; address < 127; ++address) {
    bus.beginTransmission(address);
    if (bus.endTransmission() == 0) {
      char buffer[8];
      snprintf(buffer, sizeof(buffer), "0x%02X", address);
      if (!out.isEmpty()) out += ',';
      out += buffer;
    }
  }
  return out;
}

void initBatteryPowerHold() {
  pinMode(PIN_BATTERY_HOLD, OUTPUT);
  digitalWrite(PIN_BATTERY_HOLD, HIGH);
  pinMode(PIN_BATTERY_KEY, INPUT_PULLUP);
}

void calibrateButtonIdle(ButtonTracker &button) {
  uint8_t lowCount = 0;
  for (uint8_t i = 0; i < 9; ++i) {
    if (digitalRead(button.pin) == LOW) ++lowCount;
    delay(1);
  }
  button.idleLow = lowCount >= 5;
  button.rawPressed = false;
  button.stablePressed = false;
  button.changedMs = millis();
  button.pressedMs = 0;
}

void initButton(ButtonTracker &button) {
  pinMode(button.pin, INPUT_PULLUP);
  button.initMs = millis();
  button.startupSettled = false;
  calibrateButtonIdle(button);
}

void initButtons() {
  initButton(bootButton);
  initButton(powerButton);
}

bool bootHeldForSetup() {
  pinMode(PIN_BOOT_BUTTON, INPUT_PULLUP);
  delay(20);
  return digitalRead(PIN_BOOT_BUTTON) == LOW;
}

void setBacklight(uint8_t duty) {
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 3
  static bool attached = false;
  if (!attached) {
    ledcAttach(PIN_LCD_BL, 5000, 8);
    attached = true;
  }
  ledcWrite(PIN_LCD_BL, duty);
#else
  static bool attached = false;
  constexpr uint8_t channel = 0;
  if (!attached) {
    ledcSetup(channel, 5000, 8);
    ledcAttachPin(PIN_LCD_BL, channel);
    attached = true;
  }
  ledcWrite(channel, duty);
#endif
}

uint8_t estimateScreenBatteryPercent(float volts) {
  struct Point { float volts; uint8_t pct; };
  static constexpr Point curve[] = {
      {3.20f, 0}, {3.50f, 5}, {3.60f, 10}, {3.70f, 20},
      {3.75f, 30}, {3.79f, 40}, {3.83f, 50}, {3.87f, 60},
      {3.92f, 70}, {3.97f, 80}, {4.05f, 90}, {4.20f, 100},
  };
  if (volts <= curve[0].volts) return 0;
  for (size_t i = 1; i < sizeof(curve) / sizeof(curve[0]); ++i) {
    if (volts <= curve[i].volts) {
      const float span = curve[i].volts - curve[i - 1].volts;
      const float pos = (volts - curve[i - 1].volts) / span;
      return curve[i - 1].pct + static_cast<uint8_t>((curve[i].pct - curve[i - 1].pct) * pos);
    }
  }
  return 100;
}

void updateScreenBattery() {
  uint32_t sumMv = 0;
  constexpr uint8_t samples = 16;
  for (uint8_t i = 0; i < samples; ++i) {
    sumMv += analogReadMilliVolts(PIN_BATTERY_ADC);
    delay(1);
  }
  const float pinVolts = (sumMv / static_cast<float>(samples)) / 1000.0f;
  screenBattery.previousVolts = screenBattery.volts;
  screenBattery.volts = pinVolts * BATTERY_ADC_RATIO;
  screenBattery.present = screenBattery.volts > 2.5f;
  screenBattery.deltaMv = screenBattery.previousVolts > 0.1f
                              ? static_cast<int16_t>((screenBattery.volts - screenBattery.previousVolts) * 1000.0f)
                              : 0;
  screenBattery.percent = screenBattery.present ? estimateScreenBatteryPercent(screenBattery.volts) : 0;
  screenBattery.usbCdcConnected = static_cast<bool>(Serial);
  screenBattery.inputStatus = screenBattery.usbCdcConnected ? "USB data/power present" : "external power";
  if (!screenBattery.present) screenBattery.label = "no board battery";
  else if (screenBattery.volts < 3.35f) screenBattery.label = "critical";
  else if (screenBattery.volts < 3.55f) screenBattery.label = "low";
  else if (screenBattery.volts >= 4.15f) screenBattery.label = "full";
  else screenBattery.label = "ok";
  screenBattery.updatedMs = millis();
}

void markDisplayActivity() {
  lastDisplayActivityMs = millis();
}

void wakeDisplay() {
  if (!displayReady) return;
  displayManualOff = false;
  markDisplayActivity();
  if (!displaySleeping) return;
  displaySleeping = false;
  setBacklight(settings.displayEnabled ? settings.brightness : 0);
  drawDisplay(true);
}

void sleepDisplayFromButton(const char *source) {
  displayManualOff = true;
  displaySleeping = true;
  lastButtonActionMs = millis();
  lastButtonAction = String(source) + " screen off";
  setBacklight(0);
}

void toggleDisplayFromButton(const char *source) {
  lastButtonActionMs = millis();
  if (displaySleeping || displayManualOff) {
    displayManualOff = false;
    lastButtonAction = String(source) + " screen on";
    wakeDisplay();
    drawDisplay(true);
    return;
  }
  sleepDisplayFromButton(source);
}

bool buttonPressed(const ButtonTracker &button) {
  return (digitalRead(button.pin) == LOW) != button.idleLow;
}

void pollButton(ButtonTracker &button) {
  const uint32_t now = millis();
  if (!button.startupSettled && now - button.initMs >= BUTTON_STARTUP_SETTLE_MS) {
    calibrateButtonIdle(button);
    button.startupSettled = true;
    return;
  }
  const bool pressed = buttonPressed(button);
  if (pressed != button.rawPressed) {
    button.rawPressed = pressed;
    button.changedMs = now;
  }
  if (now - button.changedMs < BUTTON_DEBOUNCE_MS || pressed == button.stablePressed) return;

  button.stablePressed = pressed;
  if (pressed) {
    button.pressedMs = now;
    return;
  }

  const uint32_t heldMs = button.pressedMs > 0 ? now - button.pressedMs : 0;
  button.pressedMs = 0;
  if (button.togglesDisplay && heldMs >= BUTTON_SHORT_MIN_MS && heldMs < BUTTON_LONG_MS) {
    toggleDisplayFromButton(button.name);
  }
}

void pollButtons() {
  pollButton(bootButton);
  pollButton(powerButton);
}

bool displayActivityPresent() {
  return mowerRunning() || packCharging();
}

uint32_t displayRefreshIntervalMs() {
  if (displaySleeping) return DISPLAY_SLEEP_IDLE_MS;
  if (displayPage == 2) return DISPLAY_CLOCK_REFRESH_MS;
  return DISPLAY_REFRESH_MS;
}

void manageDisplayPower() {
  if (!displayReady) return;
  if (!settings.displayEnabled) {
    if (!displaySleeping) {
      displaySleeping = true;
      setBacklight(0);
    }
    return;
  }
  const uint32_t now = millis();
  if (lastDisplayActivityMs == 0) lastDisplayActivityMs = now;
  if (displayManualOff) {
    if (!displaySleeping) {
      displaySleeping = true;
      setBacklight(0);
    }
    return;
  }
  if (displayActivityPresent()) {
    wakeDisplay();
    return;
  }
  if (!displaySleeping && now - lastDisplayActivityMs >= DISPLAY_SLEEP_IDLE_MS) {
    displaySleeping = true;
    setBacklight(0);
  }
}

void updateRuntime() {
  const uint32_t now = millis();
  if (lastRuntimeTickMs == 0) {
    lastRuntimeTickMs = now;
    lastRuntimeSaveMs = now;
    return;
  }
  const uint32_t delta = now - lastRuntimeTickMs;
  lastRuntimeTickMs = now;
  if (mowerRunning()) {
    runTimeRemainderMs += delta;
    const uint32_t seconds = runTimeRemainderMs / 1000;
    if (seconds > 0) {
      runtimeSeconds += seconds;
      rideSessionSeconds += seconds;
      runTimeRemainderMs %= 1000;
    }
  } else {
    runTimeRemainderMs = 0;
  }
  if (now - lastRuntimeSaveMs >= RUNTIME_SAVE_MS) saveRuntime();
}

void setRuntimeHours(float hours) {
  runtimeSeconds = static_cast<uint64_t>(max(0.0f, hours) * 3600.0f);
  saveRuntime();
}

void initDisplay() {
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  Wire.setClock(400000);
  initIoExpander();
  touchWire.begin(PIN_TOUCH_SDA, PIN_TOUCH_SCL);
  touchWire.setClock(400000);
  pinMode(PIN_TOUCH_INT, INPUT_PULLUP);
  touchReady = i2cDevicePresent(touchWire, CST816_ADDR);
  internalI2cAddresses = i2cAddressCsv(Wire);
  touchI2cAddresses = i2cAddressCsv(touchWire);
  setBacklight(settings.displayEnabled ? settings.brightness : 0);
  displayReady = gfx->begin(40000000);
  if (displayReady) {
    applyDisplayRotation();
    R48DisplayUi::begin(gfx);
    gfx->fillScreen(COLOR_BG);
    gfx->setTextWrap(false);
    markDisplayActivity();
  }
}

bool readTouch(TouchPoint &point) {
  touchWire.beginTransmission(CST816_ADDR);
  touchWire.write(0x01);
  if (touchWire.endTransmission(false) != 0) return false;
  if (touchWire.requestFrom(static_cast<int>(CST816_ADDR), 6) != 6) return false;
  point.gesture = touchWire.read();
  const uint8_t fingers = touchWire.read() & 0x0F;
  const uint8_t xHigh = touchWire.read();
  const uint8_t xLow = touchWire.read();
  const uint8_t yHigh = touchWire.read();
  const uint8_t yLow = touchWire.read();
  if (fingers == 0 && point.gesture == 0) return false;
  point.x = ((xHigh & 0x0F) << 8) | xLow;
  point.y = ((yHigh & 0x0F) << 8) | yLow;
  const uint16_t rawX = constrain(point.x, static_cast<uint16_t>(0), static_cast<uint16_t>(DISPLAY_WIDTH - 1));
  const uint16_t rawY = constrain(point.y, static_cast<uint16_t>(0), static_cast<uint16_t>(DISPLAY_HEIGHT - 1));
  switch (displayRotationStep()) {
    case 1:
      point.x = rawY;
      point.y = DISPLAY_WIDTH - 1 - rawX;
      break;
    case 2:
      point.x = DISPLAY_WIDTH - 1 - rawX;
      point.y = DISPLAY_HEIGHT - 1 - rawY;
      break;
    case 3:
      point.x = DISPLAY_HEIGHT - 1 - rawY;
      point.y = rawX;
      break;
    default:
      point.x = rawX;
      point.y = rawY;
      break;
  }
  return true;
}

void maybeProbeTouch() {
  if (touchReady || millis() - lastTouchProbeMs < 5000) return;
  lastTouchProbeMs = millis();
  touchReady = i2cDevicePresent(touchWire, CST816_ADDR);
  touchI2cAddresses = i2cAddressCsv(touchWire);
}

void maybeHandleTouch() {
  if (!displayReady || !touchReady || millis() - lastTouchMs < 300) return;
  if (digitalRead(PIN_TOUCH_INT) == HIGH) return;
  TouchPoint p;
  if (!readTouch(p)) return;
  if (displaySleeping) {
    lastTouchMs = millis();
    wakeDisplay();
    return;
  }
  markDisplayActivity();
  if (p.gesture == 0x03) displayPage = (displayPage + 1) % R48DisplayUi::PAGE_COUNT;
  else if (p.gesture == 0x04) displayPage = (displayPage + R48DisplayUi::PAGE_COUNT - 1) % R48DisplayUi::PAGE_COUNT;
  else displayPage = (displayPage + 1) % R48DisplayUi::PAGE_COUNT;
  lastTouchMs = millis();
  drawDisplay(true);
}

void autoDisplayMode() {
  const bool charging = packCharging();
  if (charging != previousCharging) {
    previousCharging = charging;
    displayPage = charging ? 1 : 0;
    wakeDisplay();
    drawDisplay(true);
  }
}

void configureClock() {
  setenv("TZ", settings.timezone.c_str(), 1);
  tzset();
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  ntpConfigured = true;
}

void startProvisioningAp() {
  WiFi.disconnect(false, false);
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_AP);
  WiFi.setSleep(true);
  esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
  apSsid = "R48Display-" + chipSuffix();
  provisioningActive = WiFi.softAP(apSsid.c_str(), settings.apPassword.c_str(), 6, false, 2);
  mdnsReady = false;
}

void startStaOnly() {
  provisioningActive = false;
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);
  WiFi.setSleep(true);
  esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
  WiFi.setHostname(settings.hostname.c_str());
  if (!settings.wifiSsid.isEmpty()) {
    WiFi.begin(settings.wifiSsid.c_str(), settings.wifiPassword.c_str());
    lastWifiAttemptMs = millis();
  }
  mdnsReady = false;
}

void setupWiFi(bool forceProvisioning) {
  apSsid = "R48Display-" + chipSuffix();
  WiFi.persistent(false);
  WiFi.setSleep(true);
  esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
  WiFi.setHostname(settings.hostname.c_str());
  if (forceProvisioning || settings.wifiSsid.isEmpty()) startProvisioningAp();
  else startStaOnly();
}

void maintainWiFi() {
  if (pendingProvisioningStart) {
    pendingProvisioningStart = false;
    startProvisioningAp();
    drawDisplay(true);
    return;
  }
  if (pendingStaStart) {
    pendingStaStart = false;
    startStaOnly();
    drawDisplay(true);
    return;
  }
  if (provisioningActive) return;
  if (!settings.wifiSsid.isEmpty() && WiFi.status() != WL_CONNECTED &&
      millis() - lastWifiAttemptMs > WIFI_RETRY_MS) {
    WiFi.begin(settings.wifiSsid.c_str(), settings.wifiPassword.c_str());
    lastWifiAttemptMs = millis();
  }
  if (WiFi.status() == WL_CONNECTED && !mdnsReady) {
    if (MDNS.begin(settings.hostname.c_str())) {
      MDNS.addService("http", "tcp", 80);
      mdnsReady = true;
    }
  }
  if (WiFi.status() == WL_CONNECTED && !ntpConfigured) configureClock();
}

void setupOta() {
  ArduinoOTA.setHostname(settings.hostname.c_str());
  ArduinoOTA.setPassword(settings.otaPassword.c_str());
  ArduinoOTA.onStart([]() {
    if (!displayReady) return;
    gfx->fillScreen(COLOR_BG);
    gfx->setTextSize(2);
    gfx->setTextColor(COLOR_YELLOW, COLOR_BG);
    gfx->setCursor(92, 160);
    gfx->print(F("OTA update"));
  });
  ArduinoOTA.onEnd([]() {
    if (!displayReady) return;
    gfx->setCursor(116, 190);
    gfx->setTextColor(COLOR_GREEN, COLOR_BG);
    gfx->print(F("rebooting"));
  });
  ArduinoOTA.begin();
}

void drawDisplay(bool fullRedraw) {
  if (!displayReady || !settings.displayEnabled) return;
  if (displaySleeping && !fullRedraw) return;
  const ThemeProfile &theme = activeTheme();
  R48DisplayUi::Snapshot s;
  s.page = displayPage;
  s.fullRedraw = fullRedraw;
  s.themeBg = theme.bg;
  s.themePanel = theme.panel;
  s.themePanel2 = theme.panel2;
  s.themeLine = theme.line;
  s.themeText = theme.text;
  s.themeMuted = theme.muted;
  s.themePrimary = theme.primary;
  s.themePrimaryDark = theme.primaryDark;
  s.themeAccent = theme.accent;
  s.themeWarn = theme.warn;
  s.themeBad = theme.bad;
  s.socValid = bms.lastAnalogMs > 0;
  s.soc = bms.soc;
  s.packVoltage = bms.packVoltage;
  s.deltaCellMv = bms.deltaCellMv;
  s.loadPercent = loadPercent();
  s.dischargeWatts = dischargeWatts();
  s.dischargeAmps = dischargeCurrentA();
  s.chargeWatts = chargeWatts();
  s.chargeAmps = chargeCurrentA();
  s.healthPercent = bmsHealthPercent();
  s.remainingAh = effectiveRemainingAh();
  s.designAh = effectiveDesignAh();
  s.mode = mowerModeLabel();
  s.runEta = formatHours(runtimeEstimateHours());
  s.chargeEta = packCharging() ? formatHours(chargeEstimateHours()) : "--";
  s.rideTime = formatDuration(rideSessionSeconds);
  s.temp = formatTemp(bms.pcbTempC);
  s.runtimeHours = String(runtimeSeconds / 3600.0f, 1) + " h";
  s.wifiConnected = WiFi.status() == WL_CONNECTED;
  s.provisioning = provisioningActive;
  s.wifiRssi = s.wifiConnected ? WiFi.RSSI() : 0;
  s.ip = s.wifiConnected ? ipToString(WiFi.localIP()) : provisioningActive ? ipToString(WiFi.softAPIP()) : "";
  s.ssid = s.wifiConnected ? WiFi.SSID() : provisioningActive ? apSsid : settings.wifiSsid;
  s.bleLink = bmsLinkLabel();
  s.bmsRssi = bms.rssi;
  s.bmsTarget = settings.bmsName.length() ? settings.bmsName : bms.name;
  s.bmsAddress = bms.address;
  s.lastError = bms.lastError;
  s.screenBattery = screenBattery.present ? String(screenBattery.volts, 2) + "V " + String(screenBattery.percent) + "%" : "no battery";
  s.touchReady = touchReady;
  s.localTime = currentTimeText("%Y-%m-%d %H:%M:%S");
  s.uptime = formatDuration(millis() / 1000ULL);
  s.firmware = FIRMWARE_VERSION;
  R48DisplayUi::draw(s);
}

String escapeHtml(const String &input) {
  String out;
  out.reserve(input.length() + 8);
  for (size_t i = 0; i < input.length(); ++i) {
    switch (input[i]) {
      case '&': out += F("&amp;"); break;
      case '<': out += F("&lt;"); break;
      case '>': out += F("&gt;"); break;
      case '"': out += F("&quot;"); break;
      case '\'': out += F("&#39;"); break;
      default: out += input[i]; break;
    }
  }
  return out;
}

String commonHead(const String &title) {
  String html = F("<!doctype html><html><head><meta charset='utf-8'>"
                  "<meta name='viewport' content='width=device-width,initial-scale=1'>"
                  "<link rel='stylesheet' href='/theme.css'><title>");
  html += escapeHtml(title);
  html += F("</title></head><body><header><div class='brand'><span class='mark'></span><div><h1>R48 Power Display</h1><p>Mowers + carts + power - ");
  html += escapeHtml(title);
  html += F("</p></div></div><nav><a href='/'>Dashboard</a><a href='/battery'>Battery</a><a href='/settings'>Settings</a><a href='/status'>Status</a><a href='/update'>Update</a></nav></header><main class='wrap'>");
  return html;
}

String commonFoot() {
  String html = F("</main><script src='/app.js'></script></body></html>");
  return html;
}

String themeCss() {
  const ThemeProfile &theme = activeTheme();
  String css;
  css.reserve(4300);
  css += F(":root{color-scheme:dark;");
  css += F("--bg:"); css += cssColor(theme.bg); css += ';';
  css += F("--panel:"); css += cssColor(theme.panel); css += ';';
  css += F("--panel2:"); css += cssColor(theme.panel2); css += ';';
  css += F("--line:"); css += cssColor(theme.line); css += ';';
  css += F("--text:"); css += cssColor(theme.text); css += ';';
  css += F("--muted:"); css += cssColor(theme.muted); css += ';';
  css += F("--primary:"); css += cssColor(theme.primary); css += ';';
  css += F("--primary2:"); css += cssColor(theme.primaryDark); css += ';';
  css += F("--accent:"); css += cssColor(theme.accent); css += ';';
  css += F("--warn:"); css += cssColor(theme.warn); css += ';';
  css += F("--bad:"); css += cssColor(theme.bad); css += ';';
  css += F("--buttonText:"); css += cssColor(theme.buttonText); css += ';';
  css += F("--font:system-ui,-apple-system,Segoe UI,sans-serif}"
           "*{box-sizing:border-box}body{margin:0;background:var(--bg);color:var(--text);font-family:var(--font);min-height:100vh}header{position:sticky;top:0;z-index:3;background:var(--bg);border-bottom:1px solid var(--line);display:flex;gap:14px;align-items:center;justify-content:space-between;padding:12px 16px}"
           "h1,h2,h3,p{margin:0}h1{font-size:20px;color:var(--primary)}header p{color:var(--muted);font-size:12px;margin-top:2px}.brand{display:flex;gap:10px;align-items:center}.mark{width:30px;height:30px;border-radius:8px;background:linear-gradient(135deg,var(--primary),var(--accent));display:block}"
           "nav{display:flex;gap:8px;flex-wrap:wrap}a,button,input,select{font:inherit}nav a,button{border:1px solid var(--line);background:var(--panel2);color:var(--text);border-radius:7px;padding:8px 11px;text-decoration:none;cursor:pointer}button.primary{background:var(--primary);border-color:var(--primary);color:var(--buttonText);font-weight:800}"
           ".wrap{max-width:1180px;margin:0 auto;padding:18px;display:grid;gap:16px}.toolbar{display:flex;gap:8px;flex-wrap:wrap}.grid,.dashboard-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(210px,1fr));gap:12px}.wide{grid-column:1/-1}.card,.metric,.gauge-card{background:var(--panel);border:1px solid var(--line);border-radius:8px;padding:14px}.metric{min-height:104px}.label{color:var(--muted);font-size:13px}.value{font-size:30px;margin-top:6px}.value.sm{font-size:20px;line-height:1.3}.subline{color:var(--muted);text-align:center;margin-top:12px}"
           ".gauge-card{display:grid;place-items:center}.gauge{width:min(70vw,292px);aspect-ratio:1;border-radius:50%;display:grid;place-items:center;position:relative;background:conic-gradient(var(--primary) calc(var(--pct)*1%),var(--line) 0)}.gauge:before{content:'';position:absolute;inset:18px;border-radius:50%;background:var(--panel)}.gauge b,.gauge span{position:relative}.gauge b{font-size:62px;color:var(--primary)}.gauge span{align-self:start;margin-top:-92px;color:var(--muted)}"
           ".section-head{display:flex;align-items:center;justify-content:space-between;gap:10px;margin-bottom:12px}.form-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(220px,1fr));gap:12px}label{display:grid;gap:6px;color:var(--muted);font-size:13px}label.check{display:flex;align-items:center;gap:8px;color:var(--text)}label.check input{width:auto}.hint{color:var(--muted);font-size:12px;line-height:1.35}input,select{width:100%;background:var(--panel2);color:var(--text);border:1px solid var(--line);border-radius:7px;padding:9px}.actions{display:flex;gap:8px;flex-wrap:wrap;margin-bottom:12px}"
           ".cells{display:grid;grid-template-columns:repeat(auto-fit,minmax(132px,1fr));gap:8px}.cell{background:var(--panel2);border:1px solid var(--line);border-radius:7px;padding:9px;display:grid;gap:6px}.cell span{color:var(--muted);font-size:12px}.cell b{font-size:18px}.cell i{display:block;height:6px;border-radius:6px;background:linear-gradient(90deg,var(--primary2),var(--primary));width:var(--w)}.list{display:grid;gap:8px}.list button{display:flex;justify-content:space-between;text-align:left}.list span{color:var(--muted)}.empty,.note{color:var(--muted);line-height:1.45}table{width:100%;border-collapse:collapse;background:var(--panel);border:1px solid var(--line)}td,th{padding:8px;border-bottom:1px solid var(--line);text-align:left;vertical-align:top}th{color:var(--muted);font-weight:500}@media(max-width:760px){header{align-items:flex-start;flex-direction:column}.gauge b{font-size:50px}}");
  return css;
}

void sendContentPieces(const String &content, size_t chunkSize = 1024) {
  for (size_t i = 0; i < content.length(); i += chunkSize) {
    server.sendContent(content.substring(i, min(i + chunkSize, content.length())));
    delay(1);
  }
}

void sendPage(const String &title, String (*body)()) {
  String head = commonHead(title);
  String bodyHtml = body();
  String foot = commonFoot();
  server.sendHeader("Connection", "close");
  server.setContentLength(head.length() + bodyHtml.length() + foot.length());
  server.send(200, "text/html", "");
  sendContentPieces(head);
  sendContentPieces(bodyHtml);
  sendContentPieces(foot);
}

void sendJson(JsonDocument &doc) {
  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

void addStatusJson(JsonDocument &doc) {
  doc["firmware"] = FIRMWARE_VERSION;
  doc["project"] = PROJECT_NAME;
  doc["uptime_ms"] = millis();
  doc["uptime"] = formatDuration(millis() / 1000ULL);
  doc["hostname"] = settings.hostname;

  JsonObject wifi = doc["wifi"].to<JsonObject>();
  const bool sta = WiFi.status() == WL_CONNECTED;
  wifi["mode"] = provisioningActive ? "setup_ap" : "sta";
  wifi["sta_connected"] = sta;
  wifi["ssid"] = sta ? WiFi.SSID() : provisioningActive ? apSsid : settings.wifiSsid;
  wifi["ip"] = sta ? ipToString(WiFi.localIP()) : "";
  wifi["rssi"] = sta ? WiFi.RSSI() : 0;
  wifi["signal"] = sta ? wifiSignalLabel(WiFi.RSSI()) : "not connected";
  wifi["ap_ssid"] = provisioningActive ? apSsid : "";
  wifi["ap_ip"] = provisioningActive ? ipToString(WiFi.softAPIP()) : "";
  wifi["mdns"] = mdnsReady ? String(settings.hostname + ".local") : "";
  wifi["provisioning"] = provisioningActive;

  const UsageCategory &usage = activeUsage();
  const ThemeProfile &theme = activeTheme();
  JsonObject themeJson = doc["theme"].to<JsonObject>();
  themeJson["id"] = theme.id;
  themeJson["label"] = theme.label;
  themeJson["family"] = theme.family;
  themeJson["description"] = theme.description;
  themeJson["primary"] = cssColor(theme.primary);
  themeJson["accent"] = cssColor(theme.accent);

  JsonObject usageJson = doc["usage"].to<JsonObject>();
  usageJson["category"] = settings.usageCategory;
  usageJson["label"] = usage.label;
  usageJson["default_theme"] = usage.defaultThemeId;
  usageJson["active_label"] = usage.activeLabel;
  usageJson["work_label"] = usage.workLabel;
  usageJson["activity_detection"] = settings.activityDetection;
  usageJson["work_detection"] = settings.workDetection;
  usageJson["audio_assist_available"] = usage.audioAssist;

  JsonObject vehicle = doc["vehicle"].to<JsonObject>();
  vehicle["label"] = settings.mowerModel;
  vehicle["usage_category"] = settings.usageCategory;
  vehicle["usage_label"] = usage.label;
  vehicle["mode"] = mowerModeLabel();
  vehicle["runtime_hours"] = serialized(String(runtimeSeconds / 3600.0f, 2));
  vehicle["active"] = activityDetected();
  vehicle["work_likely"] = workDetected();
  vehicle["audio_work_tone"] = usage.audioAssist && R48Mic::toneDetected();
  vehicle["active_label"] = usage.activeLabel;
  vehicle["work_label"] = usage.workLabel;
  vehicle["activity_detection"] = settings.activityDetection;
  vehicle["work_detection"] = settings.workDetection;
  vehicle["run_threshold_a"] = serialized(String(settings.mowerRunAmps, 1));
  vehicle["work_threshold_a"] = serialized(String(settings.bladesOnAmps, 1));
  vehicle["load_percent"] = serialized(String(loadPercent(), 1));
  vehicle["discharge_a"] = serialized(String(dischargeCurrentA(), 2));
  vehicle["charge_a"] = serialized(String(chargeCurrentA(), 2));
  vehicle["discharge_w"] = serialized(String(dischargeWatts(), 0));
  vehicle["charge_w"] = serialized(String(chargeWatts(), 0));
  vehicle["runtime_estimate_hours"] = serialized(String(runtimeEstimateHours(), 2));
  vehicle["charge_estimate_hours"] = serialized(String(chargeEstimateHours(), 2));
  vehicle["health_percent_estimate"] = serialized(String(bmsHealthPercent(), 1));

  JsonObject mower = doc["mower"].to<JsonObject>();
  mower["model"] = settings.mowerModel;
  mower["mode"] = mowerModeLabel();
  mower["runtime_hours"] = serialized(String(runtimeSeconds / 3600.0f, 2));
  mower["mower_running"] = activityDetected();
  mower["blades_likely_on"] = workDetected();
  mower["mic_mower_tone"] = usage.audioAssist && R48Mic::toneDetected();
  mower["run_threshold_a"] = serialized(String(settings.mowerRunAmps, 1));
  mower["mowing_threshold_a"] = serialized(String(settings.bladesOnAmps, 1));
  mower["work_threshold_a"] = serialized(String(settings.bladesOnAmps, 1));
  mower["load_percent"] = serialized(String(loadPercent(), 1));
  mower["discharge_a"] = serialized(String(dischargeCurrentA(), 2));
  mower["charge_a"] = serialized(String(chargeCurrentA(), 2));
  mower["discharge_w"] = serialized(String(dischargeWatts(), 0));
  mower["charge_w"] = serialized(String(chargeWatts(), 0));
  mower["runtime_estimate_hours"] = serialized(String(runtimeEstimateHours(), 2));
  mower["charge_estimate_hours"] = serialized(String(chargeEstimateHours(), 2));
  mower["health_percent_estimate"] = serialized(String(bmsHealthPercent(), 1));

  JsonObject b = doc["bms"].to<JsonObject>();
  b["target_name"] = settings.bmsName;
  b["connected_name"] = bms.name;
  b["protocol"] = settings.bmsProtocol;
  b["protocol_label"] = activeProfile().label;
  b["address"] = bms.address;
  b["status"] = bms.status;
  b["link"] = bmsLinkLabel();
  b["last_error"] = bms.lastError;
  b["connected"] = bms.connected;
  b["authenticated"] = bms.authenticated;
  b["scanning"] = bms.scanning;
  b["rssi"] = bms.rssi;
  b["frames"] = bms.frames;
  b["errors"] = bms.errors;
  b["last_analog_age"] = millisAge(bms.lastAnalogMs);
  b["software"] = bms.software;
  b["manufacturer"] = bms.manufacturer;
  b["serial"] = bms.serial;
  b["soc"] = bms.soc;
  b["pack_voltage"] = serialized(String(bms.packVoltage, 2));
  b["raw_current_a"] = serialized(String(bms.currentA, 2));
  b["raw_remaining_ah"] = serialized(String(bms.remainingAh, 1));
  b["raw_total_ah"] = serialized(String(bms.totalAh, 1));
  b["raw_design_ah"] = serialized(String(bms.designAh, 1));
  b["capacity_scale"] = serialized(String(capacityScale(), 2));
  b["remaining_ah"] = serialized(String(effectiveRemainingAh(), 1));
  b["total_ah"] = serialized(String(effectiveTotalAh(), 1));
  b["design_ah"] = serialized(String(effectiveDesignAh(), 1));
  b["cycles"] = bms.cycleCount;
  b["cell_count"] = bms.cellCount;
  b["mos_temp_c"] = serialized(String(bms.mosTempC, 1));
  b["pcb_temp_c"] = serialized(String(bms.pcbTempC, 1));
  b["mos_temp"] = serialized(String(displayTempC(bms.mosTempC), 1));
  b["pcb_temp"] = serialized(String(displayTempC(bms.pcbTempC), 1));
  b["temp_unit"] = settings.tempUnit;
  b["min_cell_v"] = serialized(String(bms.minCellV, 3));
  b["max_cell_v"] = serialized(String(bms.maxCellV, 3));
  b["delta_cell_mv"] = serialized(String(bms.deltaCellMv, 1));
  float avgCell = 0.0f;
  for (uint8_t i = 0; i < bms.cellCount && i < 32; ++i) avgCell += bms.cellVolts[i];
  if (bms.cellCount > 0) avgCell /= bms.cellCount;
  b["avg_cell_v"] = serialized(String(avgCell, 3));
  b["warning_hex"] = bms.warningHex;
  JsonArray cells = b["cells"].to<JsonArray>();
  for (uint8_t i = 0; i < bms.cellCount && i < 32; ++i) cells.add(serialized(String(bms.cellVolts[i], 3)));

  JsonObject hardware = doc["hardware"].to<JsonObject>();
  hardware["display_ready"] = displayReady;
  hardware["display_enabled"] = settings.displayEnabled;
  hardware["display_sleeping"] = displaySleeping;
  hardware["display_rotation"] = settings.displayRotation;
  hardware["boot_button_pressed"] = bootButton.stablePressed;
  hardware["power_button_pressed"] = powerButton.stablePressed;
  hardware["last_button_action"] = lastButtonAction;
  hardware["last_button_action_age"] = millisAge(lastButtonActionMs);
  hardware["touch_ready"] = touchReady;
  hardware["i2c"] = internalI2cAddresses;
  hardware["touch_i2c"] = touchI2cAddresses;
  hardware["free_heap"] = ESP.getFreeHeap();
  hardware["psram"] = psramFound() ? ESP.getFreePsram() : 0;

  const R48Mic::Snapshot micState = R48Mic::snapshot();
  JsonObject mic = doc["mic"].to<JsonObject>();
  mic["enabled"] = micState.enabled;
  mic["ready"] = micState.ready;
  mic["tone"] = micState.tone;
  mic["rms"] = micState.rms;
  mic["peak"] = micState.peak;
  mic["floor"] = micState.floor;
  mic["detections"] = micState.detections;
  mic["errors"] = micState.errors;
  mic["last_read_age"] = millisAge(micState.lastReadMs);
  mic["threshold"] = serialized(String(micState.threshold, 0));
  mic["status"] = micState.status;

  JsonObject display = doc["display"].to<JsonObject>();
  display["page"] = displayPage;
  display["page_name"] = displayPageName(displayPage);
  display["rotation_degrees"] = settings.displayRotation;
  display["refresh_ms"] = displayRefreshIntervalMs();
  display["sleep_timeout_ms"] = DISPLAY_SLEEP_IDLE_MS;

  JsonObject espBat = doc["screen_battery"].to<JsonObject>();
  espBat["present"] = screenBattery.present;
  espBat["voltage"] = serialized(String(screenBattery.volts, 3));
  espBat["percent"] = screenBattery.percent;
  espBat["label"] = screenBattery.label;
  espBat["input_status"] = screenBattery.inputStatus;

  JsonObject clock = doc["clock"].to<JsonObject>();
  clock["ntp_configured"] = ntpConfigured;
  clock["timezone"] = settings.timezone;
  clock["local_time"] = currentTimeText("%Y-%m-%d %H:%M:%S");
}

void apiStatus() {
  JsonDocument doc;
  addStatusJson(doc);
  sendJson(doc);
}

void apiSettingsGet() {
  JsonDocument doc;
  doc["hostname"] = settings.hostname;
  doc["ap_ssid"] = apSsid;
  doc["ap_password"] = settings.apPassword;
  doc["ota_password"] = settings.otaPassword;
  doc["wifi_ssid"] = settings.wifiSsid;
  doc["bms_name"] = settings.bmsName;
  doc["bms_protocol"] = settings.bmsProtocol;
  doc["mower_model"] = settings.mowerModel;
  doc["usage_category"] = settings.usageCategory;
  doc["vehicle_type"] = settings.usageCategory;
  doc["theme_id"] = settings.themeId;
  doc["discharge_current_negative"] = settings.dischargeCurrentNegative;
  doc["display_enabled"] = settings.displayEnabled;
  doc["activity_detection"] = settings.activityDetection;
  doc["work_detection"] = settings.workDetection;
  doc["typical_mow_amps"] = settings.typicalMowAmps;
  doc["mower_run_amps"] = settings.mowerRunAmps;
  doc["mowing_detect_amps"] = settings.bladesOnAmps;
  doc["nominal_pack_ah"] = settings.nominalPackAh;
  doc["temp_unit"] = settings.tempUnit;
  doc["timezone"] = settings.timezone;
  doc["brightness"] = settings.brightness;
  doc["display_rotation"] = settings.displayRotation;
  doc["feature_mic"] = settings.featureMic;
  doc["mic_run_threshold"] = serialized(String(settings.micRunThreshold, 0));
  doc["runtime_hours"] = serialized(String(runtimeSeconds / 3600.0f, 2));
  sendJson(doc);
}

void apiSettingsPost() {
  const String oldBms = settings.bmsName;
  const String oldProto = settings.bmsProtocol;
  const String oldSsid = settings.wifiSsid;
  const uint16_t oldRotation = settings.displayRotation;
  const String oldUsage = settings.usageCategory;
  if (server.hasArg("hostname")) settings.hostname = sanitizeHostname(server.arg("hostname"));
  if (server.hasArg("ota_password") && server.arg("ota_password").length() >= 8) settings.otaPassword = server.arg("ota_password");
  if (server.hasArg("wifi_ssid")) settings.wifiSsid = server.arg("wifi_ssid");
  if (server.hasArg("wifi_password") && server.arg("wifi_password").length() > 0) settings.wifiPassword = server.arg("wifi_password");
  if (server.hasArg("bms_name")) settings.bmsName = server.arg("bms_name");
  if (server.hasArg("bms_protocol")) settings.bmsProtocol = server.arg("bms_protocol");
  if (server.hasArg("mower_model")) settings.mowerModel = server.arg("mower_model");
  if (server.hasArg("usage_category")) settings.usageCategory = server.arg("usage_category");
  else if (server.hasArg("vehicle_type")) settings.usageCategory = server.arg("vehicle_type");
  if (server.hasArg("theme_id")) settings.themeId = server.arg("theme_id");
  if (server.hasArg("discharge_current_negative")) settings.dischargeCurrentNegative = server.arg("discharge_current_negative") == "1";
  if (server.hasArg("display_enabled")) settings.displayEnabled = server.arg("display_enabled") == "1";
  if (server.hasArg("activity_detection")) settings.activityDetection = server.arg("activity_detection") == "1";
  if (server.hasArg("work_detection")) settings.workDetection = server.arg("work_detection") == "1";
  if (server.hasArg("typical_mow_amps")) settings.typicalMowAmps = constrain(server.arg("typical_mow_amps").toFloat(), 5.0f, 160.0f);
  if (server.hasArg("mower_run_amps")) settings.mowerRunAmps = constrain(server.arg("mower_run_amps").toFloat(), 1.0f, 80.0f);
  if (server.hasArg("mowing_detect_amps")) settings.bladesOnAmps = constrain(server.arg("mowing_detect_amps").toFloat(), settings.mowerRunAmps, 200.0f);
  if (server.hasArg("nominal_pack_ah")) settings.nominalPackAh = constrain(server.arg("nominal_pack_ah").toFloat(), 1.0f, 400.0f);
  if (server.hasArg("timezone")) settings.timezone = server.arg("timezone");
  if (server.hasArg("brightness")) settings.brightness = constrain(static_cast<uint8_t>(server.arg("brightness").toInt()), static_cast<uint8_t>(20), static_cast<uint8_t>(255));
  if (server.hasArg("display_rotation")) settings.displayRotation = normalizeDisplayRotation(server.arg("display_rotation").toInt());
  if (server.hasArg("feature_mic")) settings.featureMic = server.arg("feature_mic") == "1";
  if (server.hasArg("mic_run_threshold")) settings.micRunThreshold = constrain(server.arg("mic_run_threshold").toFloat(), 100.0f, 12000.0f);
  if (server.hasArg("runtime_hours")) setRuntimeHours(server.arg("runtime_hours").toFloat());
  if (!validUsageCategory(settings.usageCategory)) settings.usageCategory = USAGE_CATEGORIES[0].id;
  if (!server.hasArg("theme_id") && oldUsage != settings.usageCategory) settings.themeId = activeUsage().defaultThemeId;
  if (!validThemeId(settings.themeId)) settings.themeId = activeUsage().defaultThemeId;
  if (settings.bladesOnAmps < settings.mowerRunAmps) settings.bladesOnAmps = settings.mowerRunAmps;
  if (settings.apPassword.length() < 8) settings.apPassword = "r48display";
  if (settings.otaPassword.length() < 8) settings.otaPassword = "r48display";
  saveSettings();
  R48Mic::configure(settings.featureMic && activeUsage().audioAssist, settings.micRunThreshold);
  if (oldRotation != settings.displayRotation && displayReady) {
    applyDisplayRotation();
    gfx->fillScreen(COLOR_BG);
  }
  setBacklight(settings.displayEnabled ? settings.brightness : 0);
  configureClock();
  if (oldBms != settings.bmsName || oldProto != settings.bmsProtocol) bleBms.reconnect();
  if (oldSsid != settings.wifiSsid && !settings.wifiSsid.isEmpty()) pendingStaStart = true;
  drawDisplay(true);
  server.send(200, "application/json", "{\"ok\":true}");
}

void apiBmsProfiles() {
  JsonDocument doc;
  JsonArray profiles = doc["profiles"].to<JsonArray>();
  for (const auto &profile : BMS_PROFILES) {
    JsonObject item = profiles.add<JsonObject>();
    item["id"] = profile.id;
    item["label"] = profile.label;
    item["service_uuid"] = profile.serviceUuid;
    item["notify_uuid"] = profile.notifyUuid;
    item["write_uuid"] = profile.writeUuid;
  }
  sendJson(doc);
}

void apiThemes() {
  JsonDocument doc;
  JsonArray themes = doc["themes"].to<JsonArray>();
  for (const auto &theme : THEME_PROFILES) {
    JsonObject item = themes.add<JsonObject>();
    item["id"] = theme.id;
    item["label"] = theme.label;
    item["family"] = theme.family;
    item["description"] = theme.description;
    item["primary"] = cssColor(theme.primary);
    item["accent"] = cssColor(theme.accent);
  }
  sendJson(doc);
}

void apiUsageCategories() {
  JsonDocument doc;
  JsonArray categories = doc["categories"].to<JsonArray>();
  for (const auto &usage : USAGE_CATEGORIES) {
    JsonObject item = categories.add<JsonObject>();
    item["id"] = usage.id;
    item["label"] = usage.label;
    item["default_theme"] = usage.defaultThemeId;
    item["active_label"] = usage.activeLabel;
    item["work_label"] = usage.workLabel;
    item["active_mode"] = usage.activeMode;
    item["work_mode"] = usage.workMode;
    item["audio_assist"] = usage.audioAssist;
  }
  sendJson(doc);
}

void apiWifiScan() {
  const wifi_mode_t oldMode = WiFi.getMode();
  const int count = WiFi.scanNetworks(false, true);
  JsonDocument doc;
  JsonArray networks = doc["networks"].to<JsonArray>();
  for (int i = 0; i < count; ++i) {
    JsonObject obj = networks.add<JsonObject>();
    obj["ssid"] = WiFi.SSID(i);
    obj["rssi"] = WiFi.RSSI(i);
    obj["bssid"] = WiFi.BSSIDstr(i);
    obj["channel"] = WiFi.channel(i);
    obj["secure"] = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;
  }
  WiFi.scanDelete();
  if (oldMode == WIFI_AP) WiFi.mode(WIFI_AP);
  sendJson(doc);
}

void apiWifiForget() {
  settings.wifiSsid = "";
  settings.wifiPassword = "";
  saveSettings();
  WiFi.disconnect(false, false);
  pendingProvisioningStart = true;
  server.send(200, "application/json", "{\"ok\":true,\"note\":\"setup AP starting\"}");
}

void apiProvisioningStart() {
  pendingProvisioningStart = true;
  server.send(200, "application/json", "{\"ok\":true,\"note\":\"setup AP starting\"}");
}

void apiBleScan() {
  if (!bms.initialized) bleBms.begin();
  const BmsProfile &profile = activeProfile();
  NimBLEScan *scan = NimBLEDevice::getScan();
  scan->clearResults();
  NimBLEScanResults results = scan->getResults(3000, false);
  JsonDocument doc;
  JsonArray devices = doc["devices"].to<JsonArray>();
  for (int i = 0; i < results.getCount(); ++i) {
    const NimBLEAdvertisedDevice *d = results.getDevice(i);
    if (!d) continue;
    JsonObject obj = devices.add<JsonObject>();
    obj["name"] = String(d->getName().c_str());
    obj["address"] = String(d->getAddress().toString().c_str());
    obj["rssi"] = d->getRSSI();
    obj["compatible"] = deviceAdvertisesProfile(d, profile);
    const BmsProfile *recommended = firstCompatibleProfile(d);
    obj["recommended_profile"] = recommended ? recommended->id : "";
    obj["recommended_label"] = recommended ? recommended->label : "";
    JsonArray profileIds = obj["compatible_profiles"].to<JsonArray>();
    for (const auto &candidate : BMS_PROFILES) {
      if (deviceAdvertisesProfile(d, candidate)) profileIds.add(candidate.id);
    }
  }
  scan->clearResults();
  sendJson(doc);
}

void apiBmsReconnect() {
  bleBms.reconnect();
  server.send(200, "application/json", "{\"ok\":true}");
}

void apiBmsReadNow() {
  const bool ok = bleBms.readNow();
  server.send(ok ? 200 : 409, "application/json", ok ? "{\"ok\":true}" : "{\"ok\":false,\"error\":\"BMS not connected\"}");
}

void apiDisplayPage() {
  if (!server.hasArg("page")) {
    server.send(400, "application/json", "{\"ok\":false,\"error\":\"missing page\"}");
    return;
  }
  const int page = server.arg("page").toInt();
  if (page < 0 || page >= R48DisplayUi::PAGE_COUNT) {
    server.send(400, "application/json", "{\"ok\":false,\"error\":\"page out of range\"}");
    return;
  }
  displayPage = static_cast<uint8_t>(page);
  wakeDisplay();
  drawDisplay(true);
  String out = "{\"ok\":true,\"page\":";
  out += displayPage;
  out += ",\"page_name\":\"";
  out += displayPageName(displayPage);
  out += "\"}";
  server.send(200, "application/json", out);
}

void apiDisplayNext() {
  displayPage = (displayPage + 1) % R48DisplayUi::PAGE_COUNT;
  wakeDisplay();
  drawDisplay(true);
  server.send(200, "application/json", "{\"ok\":true}");
}

void apiReboot() {
  server.send(200, "application/json", "{\"ok\":true}");
  delay(250);
  ESP.restart();
}

void handleUpdateGet() {
  String html = commonHead("Firmware Update");
  html += R48Web::updateBody();
  html += commonFoot();
  server.sendHeader("Connection", "close");
  server.setContentLength(html.length());
  server.send(200, "text/html", "");
  sendContentPieces(html);
}

void handleUpdatePostDone() {
  const bool ok = !Update.hasError();
  server.sendHeader("Connection", "close");
  server.send(ok ? 200 : 500, "text/plain", ok ? "Update complete. Rebooting." : "Update failed.");
  delay(500);
  if (ok) ESP.restart();
}

void handleUpdateUpload() {
  HTTPUpload &upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) Update.printError(Serial);
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) Update.printError(Serial);
  } else if (upload.status == UPLOAD_FILE_END) {
    if (!Update.end(true)) Update.printError(Serial);
  }
}

void setupRoutes() {
  server.on("/", HTTP_GET, []() { sendPage("Dashboard", R48Web::dashboardBody); });
  server.on("/battery", HTTP_GET, []() { sendPage("Battery", R48Web::batteryBody); });
  server.on("/settings", HTTP_GET, []() { sendPage("Settings", R48Web::settingsBody); });
  server.on("/status", HTTP_GET, []() { sendPage("Status", R48Web::statusBody); });
  server.on("/update", HTTP_GET, handleUpdateGet);
  server.on("/update", HTTP_POST, handleUpdatePostDone, handleUpdateUpload);
  server.on("/theme.css", HTTP_GET, []() { server.send(200, "text/css", themeCss()); });
  server.on("/app.js", HTTP_GET, []() { server.send(200, "application/javascript", R48Web::appScript()); });
  server.on("/api/status", HTTP_GET, apiStatus);
  server.on("/api/settings", HTTP_GET, apiSettingsGet);
  server.on("/api/settings", HTTP_POST, apiSettingsPost);
  server.on("/api/bms/profiles", HTTP_GET, apiBmsProfiles);
  server.on("/api/themes", HTTP_GET, apiThemes);
  server.on("/api/usage-categories", HTTP_GET, apiUsageCategories);
  server.on("/api/wifi/scan", HTTP_GET, apiWifiScan);
  server.on("/api/wifi/forget", HTTP_POST, apiWifiForget);
  server.on("/api/provisioning/start", HTTP_POST, apiProvisioningStart);
  server.on("/api/ble/scan", HTTP_GET, apiBleScan);
  server.on("/api/bms/reconnect", HTTP_POST, apiBmsReconnect);
  server.on("/api/bms/read-now", HTTP_POST, apiBmsReadNow);
  server.on("/api/display/next", HTTP_POST, apiDisplayNext);
  server.on("/api/display/page", HTTP_POST, apiDisplayPage);
  server.on("/api/reboot", HTTP_POST, apiReboot);
  server.onNotFound([]() {
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
  });
  server.begin();
}

}  // namespace

void setup() {
  const bool setupButtonHeld = bootHeldForSetup();
  initBatteryPowerHold();
  initButtons();
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println(F("R48Display booting"));
  if (psramFound()) {
    heap_caps_malloc_extmem_enable(4096);
    Serial.printf("PSRAM enabled: %u bytes\n", static_cast<unsigned>(ESP.getPsramSize()));
  } else {
    Serial.println(F("PSRAM not detected"));
  }
  loadSettings();
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  Wire.setClock(400000);
  initDisplay();
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_BATTERY_ADC, ADC_11db);
  updateScreenBattery();
  R48Mic::begin(settings.featureMic && activeUsage().audioAssist, settings.micRunThreshold);
  bleBms.begin();
  setupWiFi(setupButtonHeld);
  if (WiFi.status() == WL_CONNECTED) configureClock();
  setupOta();
  setupRoutes();
  drawDisplay(true);
  Serial.printf("WiFi mode: %s\n", provisioningActive ? "setup AP" : "STA");
  Serial.printf("Touch %s\n", touchReady ? "ready" : "missing");
}

void loop() {
  server.handleClient();
  ArduinoOTA.handle();
  maintainWiFi();
  bleBms.loop();
  R48Mic::loop(activityDetected());
  updateRuntime();
  maybeProbeTouch();
  maybeHandleTouch();
  pollButtons();
  R48DisplayUi::tick();
  autoDisplayMode();
  manageDisplayPower();

  const uint32_t now = millis();
  if (now - lastBatteryMs >= BATTERY_REFRESH_MS) {
    lastBatteryMs = now;
    updateScreenBattery();
  }
  if (now - lastDisplayMs >= displayRefreshIntervalMs()) {
    lastDisplayMs = now;
    drawDisplay(false);
  }
}
