#include <Arduino.h>
#include <ArduinoJson.h>
#include <map>
#include <vector>
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
#include <esp_system.h>
#include <esp_wifi.h>
#include <nvs_flash.h>

#include <Arduino_GFX_Library.h>
#include <NimBLEDevice.h>

#include "BoardConfig.h"
#include "DisplayUi.h"
#include "MicDetector.h"
#include "MqttClient.h"
#include "WebPages.h"

namespace {

constexpr uint16_t COLOR_BG = 0x0000;
constexpr uint16_t COLOR_GREEN = 0x07E0;
constexpr uint16_t COLOR_YELLOW = 0xFFE0;

constexpr uint32_t WIFI_RETRY_MS = 30000;
constexpr uint32_t DISPLAY_REFRESH_MS = 2500;
constexpr uint32_t DISPLAY_CLOCK_REFRESH_MS = 1000;
constexpr uint32_t DISPLAY_SLEEP_IDLE_MS = 300000;
constexpr uint32_t DISPLAY_SLEEP_BATTERY_CAP_MS = 120000;
constexpr uint32_t BUTTON_DEBOUNCE_MS = 45;
constexpr uint32_t BUTTON_SHORT_MIN_MS = 35;
constexpr uint32_t BUTTON_LONG_MS = 1200;
constexpr uint32_t BUTTON_STARTUP_SETTLE_MS = 2500;
constexpr uint32_t TOUCH_DOUBLE_TAP_MS = 450;
constexpr uint32_t BATTERY_REFRESH_MS = 30000;
constexpr float BATTERY_EXTERNAL_HIGH_V = 4.12f;
constexpr float BATTERY_EXTERNAL_RISE_V = 0.050f;
constexpr int16_t BATTERY_EXTERNAL_FAST_RISE_MV = 80;
constexpr int16_t BATTERY_EXTERNAL_DROP_MV = -80;
constexpr uint32_t BLE_SCAN_RETRY_MS = 15000;
constexpr uint32_t BLE_POLL_ANALOG_MS = 5000;
constexpr uint32_t BLE_POLL_ANALOG_IDLE_MS = 60000;
constexpr uint32_t BLE_POLL_WARNING_MS = 30000;
constexpr uint32_t BLE_POLL_CELLS_MS = 15000;
constexpr uint32_t BMS_STALE_MS = 120000;
constexpr uint32_t RUNTIME_SAVE_MS = 60000;
constexpr const char *DEFAULT_SUBTITLE = "Fresh electrons, suspiciously organized.";
constexpr const char *DEFAULT_TZ = "PST8PDT,M3.2.0/2,M11.1.0/2";
constexpr const char *DEFAULT_NTP_SERVER = "pool.ntp.org";

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
constexpr uint8_t JK_CMD_CELL_INFO   = 0x96;
constexpr uint8_t JK_CMD_DEVICE_INFO = 0x97;

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
  Jk,
  Daly,
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
    {"jk_bms_ble", "JK BMS (Jikong) BLE",
     JBD_FFE0_SERVICE_UUID, JBD_FFE1_DATA_UUID, JBD_FFE1_DATA_UUID, nullptr, nullptr, false,
     BmsParser::Jk},
    {"daly_bms_ble", "Daly Smart BMS BLE",
     BMS_BLE_SERVICE_UUID, BMS_BLE_NOTIFY_UUID, BMS_BLE_WRITE_UUID, nullptr, nullptr, false,
     BmsParser::Daly},
};

constexpr ThemeProfile THEME_PROFILES[] = {
    {"chlorophyll_shift", "Chlorophyll Shift", "green",
     "Vibrant Ryobi-style lime green with high contrast battery telemetry.",
     0x020700, 0x071306, 0x10220A, 0x4F7F19,
     0xFAFFF2, 0xC4ECA7, 0xCCFF00, 0x6FDB00, 0x00F060, 0xFFE500, 0xFF3355, 0x081000},
    {"redline_charge", "Redline Charge", "red",
     "Bright red scheme for carts, utility haulers, and performance builds.",
     0x090103, 0x1C0509, 0x330912, 0x7A1B25,
     0xFFF4F4, 0xFFC0C7, 0xFF1744, 0xFF5A00, 0xFFE600, 0x00F5D4, 0xFF00A8, 0x180004},
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
    {"fabulous", "Fabulous!", "neon",
     "Vegas strip neon palette — hot pink, electric cyan, neon yellow on deep purple-black.",
     0x0A0012, 0x14001E, 0x1E0030, 0x5C0070,
     0xFFFFFF, 0xFFB8FF, 0xFF0090, 0xCC006C, 0x00F0FF, 0xFFEE00, 0xFF3300, 0x0A0012},
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
    {"custom", "Custom Large Pack", "fabulous", "activity", "high load", "active", "high load", "standby", false},
};

struct AppSettings {
  String hostname;
  String title;     // display title; empty = derive from hostname
  String apPassword = "r48display";
  String otaPassword = "r48display";
  String wifiSsid;
  String wifiPassword;
  String bmsName;
  String bmsProtocol = "humsienk_watt";
  String mowerModel = "48V battery system";
  String subtitle = DEFAULT_SUBTITLE;
  String usageCategory = "mower";
  String themeId = "chlorophyll_shift";
  bool dischargeCurrentNegative = true;
  bool displayEnabled = true;
  bool activityDetection = true;
  bool workDetection = true;
  float typicalMowAmps = DEFAULT_TYPICAL_MOW_AMPS;
  float mowerRunAmps = DEFAULT_MOWER_ON_AMPS;
  float bladesOnAmps = DEFAULT_BLADES_ON_AMPS;
  float chargeMinAmps = 0.5f;
  float nominalPackAh = 100.0f;
  String tempUnit = "F";
  String timeFormat = "12h";  // "12h" or "24h"
  String timezone = DEFAULT_TZ;
  uint8_t brightness = 210;
  uint16_t displayRotation = 0;
  uint16_t lcdTimeoutSec = 300;
  bool powerSaveEnabled = false;
  float idleBleWakeHours = 6.0f;
  float lowVoltageFloorV = 3.0f;
  uint8_t boardBatteryLowPct = 20;
  // MQTT
  bool mqttEnabled = false;
  String mqttHost;
  uint16_t mqttPort = 1883;
  String mqttUser;
  String mqttPassword;
  String mqttTopicPrefix;
  bool featureMic = false;
  float micRunThreshold = 900.0f;
  float hoursBaseline = 0.0f;
  // User-defined state labels (populated from category defaults on first use)
  String labelCharging;
  String labelStandby;
  String labelActive;
  String labelWorking;
  // NTP settings — opt-in only; no WAN requests unless explicitly enabled
  bool ntpEnabled = true;
  String ntpServer = DEFAULT_NTP_SERVER;
  bool advertiseApCredentials = true;  // cycle AP SSID/pass on LCD status line in AP mode
};

struct BatterySample {
  float volts = 0.0f;
  float previousVolts = 0.0f;
  int16_t deltaMv = 0;
  uint8_t percent = 0;
  String label = "unknown";
  String inputStatus = "unknown";
  bool present = false;
  bool usbCdcConnected = false;  // broad: SOF or battery-level inference
  bool usbSofDetected = false;   // strict: SOF-only (USB host actively connected)
  uint32_t updatedMs = 0;
};

enum class PowerSource : uint8_t {
  External,
  Battery,
  Unknown,
};

enum class BlePolicyMode : uint8_t {
  Full,
  Charging,
  Idle,
  ActiveSlow,
  ActiveMedium,
  ActiveFast,
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
  uint8_t balanceState = 0;
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

struct DegradationData {
  float maxCellSpreadMv = 0.0f;
  float minCellVoltageV = 9.9f;
  float maxTempC = -99.0f;
  uint32_t lowVoltageEvents = 0;
  uint32_t highCurrentEvents = 0;
} degradation;

AppSettings settings;
MqttClient mqttClient;
BatterySample screenBattery;
float screenBatteryBootVolts = 0.0f;
float screenBatteryLowVolts = 0.0f;
uint8_t screenBatteryRiseSamples = 0;
bool screenBatteryExternalLatched = false;
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
uint32_t lastWebRequestMs = 0;
uint32_t lastBatteryMs = 0;
uint32_t lastTouchProbeMs = 0;
uint32_t lastTouchMs = 0;
uint32_t lastSleepTapMs = 0;
uint32_t lastDisplayActivityMs = 0;
PowerSource powerSource = PowerSource::Unknown;
BlePolicyMode blePolicyMode = BlePolicyMode::Full;
float socRatePctPerHour = 0.0f;
uint8_t lastSocForRate = 0;
uint32_t lastSocRateMs = 0;
uint32_t observedAnalogMs = 0;
uint32_t lastBmsCacheSaveMs = 0;
uint32_t cachedAnalogSourceMs = 0;
bool bmsCacheLoaded = false;
float hoursTotal = 0.0f;
float hoursStandby = 0.0f;
float hoursActive = 0.0f;
float hoursWorking = 0.0f;
float sessionActiveHours = 0.0f;
uint32_t lastHoursTickMs = 0;
uint32_t lastHoursSaveMs = 0;
bool previousCharging = false;
String lastButtonAction;
uint32_t lastButtonActionMs = 0;
ButtonTracker bootButton{static_cast<uint8_t>(PIN_BOOT_BUTTON), "BOOT", true};
ButtonTracker powerButton{static_cast<uint8_t>(PIN_BATTERY_KEY), "POWER", true};

String escapeHtml(const String &input);
void drawDisplay(bool fullRedraw = false);
void setupRoutes();
void saveSettings();
void saveHours();
void loadBmsCache();
void saveBmsCache();
void sendJson(JsonDocument &doc);
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

String displayTitle() {
  if (!settings.title.isEmpty()) return settings.title;
  String t = settings.hostname;
  t.replace('_', ' ');
  t.replace('-', ' ');
  t.trim();
  return t.length() ? t : String(PROJECT_NAME);
}

const char *powerSourceName(PowerSource source) {
  switch (source) {
    case PowerSource::External: return "external";
    case PowerSource::Battery: return "internal_battery";
    default: return "unknown";
  }
}

const char *blePolicyName(BlePolicyMode mode) {
  switch (mode) {
    case BlePolicyMode::Charging: return "charging";
    case BlePolicyMode::Idle: return "idle_sleep";
    case BlePolicyMode::ActiveSlow: return "active_slow";
    case BlePolicyMode::ActiveMedium: return "active_medium";
    case BlePolicyMode::ActiveFast: return "active_fast";
    default: return "full";
  }
}

bool batteryPowerSaveActive() {
  if (settings.powerSaveEnabled) return true;
  return screenBattery.present && screenBattery.percent < settings.boardBatteryLowPct;
}

uint32_t displaySleepTimeoutMs() {
  if (!batteryPowerSaveActive()) return 0;
  uint32_t timeout = static_cast<uint32_t>(settings.lcdTimeoutSec) * 1000UL;
  if (timeout == 0 || timeout > DISPLAY_SLEEP_BATTERY_CAP_MS) return DISPLAY_SLEEP_BATTERY_CAP_MS;
  return timeout;
}

void updateSocRate() {
  if (bms.lastAnalogMs == 0 || bms.lastAnalogMs == observedAnalogMs) return;
  observedAnalogMs = bms.lastAnalogMs;
  if (bms.connected) bmsCacheLoaded = false;
  const uint32_t now = millis();
  if (lastSocRateMs > 0 && now > lastSocRateMs) {
    const float elapsedH = (now - lastSocRateMs) / 3600000.0f;
    if (elapsedH > 0.001f) {
      socRatePctPerHour = (static_cast<float>(lastSocForRate) - static_cast<float>(bms.soc)) / elapsedH;
      if (socRatePctPerHour < 0.0f) socRatePctPerHour = 0.0f;
    }
  }
  lastSocForRate = bms.soc;
  lastSocRateMs = now;
}

bool deviceAdvertisesProfile(const NimBLEAdvertisedDevice *device, const BmsProfile &profile) {
  return device && device->isAdvertisingService(NimBLEUUID(profile.serviceUuid));
}

const BmsProfile *firstCompatibleProfile(const NimBLEAdvertisedDevice *device) {
  if (!device) return nullptr;
  const String name(device->getName().c_str());
  // Name-pattern hints override UUID-order for devices that share service UUIDs
  if (name.startsWith("HS") || name.startsWith("Humsienk") || name.startsWith("humsienk")) {
    for (const auto &p : BMS_PROFILES) {
      if (String(p.id) == "humsienk_watt" && deviceAdvertisesProfile(device, p)) return &p;
    }
  }
  if (name.startsWith("JK") || name.startsWith("jk-") || name.startsWith("JK-")) {
    for (const auto &p : BMS_PROFILES) {
      if (String(p.id) == "jk_bms_ble" && deviceAdvertisesProfile(device, p)) return &p;
    }
  }
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
  return chargeCurrentA() >= settings.chargeMinAmps;
}

enum class ActivityState : uint8_t { Charging, Standby, Active, Working };

enum class MaintType : uint8_t { HoursTotal = 0, HoursActive = 1, HoursWorking = 2, Days = 3 };

struct MaintenanceItem {
  uint8_t  id = 0;
  String   name;
  MaintType type = MaintType::HoursActive;
  float    interval = 100.0f;
  uint32_t lastResetTs = 0;
  float    lastResetVal = 0.0f;
  String   notes;
};

std::vector<MaintenanceItem> maintenanceItems;

struct MaintenanceHistoryEntry {
  uint32_t ts  = 0;
  float    val = 0.0f;
  String   notes;
};

static constexpr uint8_t MAINT_HISTORY_MAX = 10;
std::map<uint8_t, std::vector<MaintenanceHistoryEntry>> maintHistory;

// Forward declarations — activityDetected/workDetected defined below
bool activityDetected();
bool workDetected();

ActivityState activityState() {
  if (packCharging()) return ActivityState::Charging;
  if (workDetected()) return ActivityState::Working;
  if (activityDetected()) return ActivityState::Active;
  return ActivityState::Standby;
}

const String &stateLabel(ActivityState state) {
  switch (state) {
    case ActivityState::Charging: return settings.labelCharging;
    case ActivityState::Active:   return settings.labelActive;
    case ActivityState::Working:  return settings.labelWorking;
    default:                      return settings.labelStandby;
  }
}

const char *activityStateKey(ActivityState state) {
  switch (state) {
    case ActivityState::Charging: return "charging";
    case ActivityState::Active:   return "active";
    case ActivityState::Working:  return "working";
    default:                      return "standby";
  }
}

bool activityDetected() {
  const float amps = dischargeCurrentA();
  if (!settings.activityDetection) return amps >= 0.2f;
  return amps >= settings.mowerRunAmps;
}

bool workDetected() {
  if (!settings.workDetection) return false;
  if (dischargeCurrentA() < settings.bladesOnAmps) return false;
  if (settings.featureMic && !R48Mic::toneDetected()) return false;
  return true;
}

bool mowerRunning() {
  return activityDetected();
}

bool bladesLikelyOn() {
  return workDetected();
}

String mowerModeLabel() {
  return stateLabel(activityState());
}

const char *displayPageName(uint8_t page) {
  switch (page % R48DisplayUi::PAGE_COUNT) {
    case 0: return "dashboard";
    case 1: return "battery";
    case 2: return "clock";
    default: return "status";
  }
}

String formatCountdown(uint32_t ms) {
  const uint32_t s = ms / 1000;
  const uint32_t h = s / 3600;
  const uint32_t m = (s % 3600) / 60;
  if (h > 0) return "in " + String(h) + "h " + String(m) + "m";
  if (m > 0) return "in " + String(m) + "m";
  return "soon";
}

String bmsLinkLabel() {
  if (bms.status.startsWith("BLE sleeping")) return "sleeping";
  if (bms.connected && bms.lastAnalogMs > 0 && millis() - bms.lastAnalogMs > BMS_STALE_MS) return "stale";
  if (bms.connected && bms.authenticated && bms.lastAnalogMs > 0) return "online";
  if (bms.connected) return "connected";
  if (!bms.connected && bms.lastAnalogMs > 0 && millis() - bms.lastAnalogMs <= BMS_STALE_MS) return "cached";
  if (bms.scanning) return "scanning";
  return "searching";
}

class BleBmsClient;
BleBmsClient *bleClientForNotify = nullptr;

class BleBmsClient {
  struct PollPolicy {
    BlePolicyMode mode = BlePolicyMode::Full;
    uint32_t analogMs = BLE_POLL_ANALOG_MS;
    uint32_t cellMs = BLE_POLL_CELLS_MS;
    uint32_t warningMs = BLE_POLL_WARNING_MS;
    bool keepConnected = true;
  };

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
    const PollPolicy policy = currentPolicy();
    blePolicyMode = policy.mode;
    if (!bms.connected) {
      if (bms.lastAnalogPollMs > 0 && now - bms.lastAnalogPollMs < policy.analogMs) {
        const uint32_t elapsed = now - bms.lastAnalogPollMs;
        const uint32_t remaining = elapsed < policy.analogMs ? policy.analogMs - elapsed : 0;
        bms.status = String("BLE sleeping — next check ") + formatCountdown(remaining);
        return;
      }
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
    if (!policy.keepConnected && bms.lastAnalogPollMs > 0 && now - bms.lastAnalogPollMs > 3000UL) {
      resetClient(true);
      const uint32_t elapsed = now - bms.lastAnalogPollMs;
      const uint32_t remaining = elapsed < policy.analogMs ? policy.analogMs - elapsed : 0;
      bms.status = String("BLE sleeping — next check ") + formatCountdown(remaining);
      return;
    }
    const uint32_t analogInterval = policy.analogMs;
    if (bms.lastAnalogMs == 0 || now - bms.lastAnalogPollMs >= analogInterval || bms.lastAnalogPollMs == 0) {
      requestAnalog();
      bms.lastAnalogPollMs = now;
      return;
    }
    if (activeProfile().parser == BmsParser::Jbd &&
        (bms.cellCount == 0 || now - bms.lastCellPollMs >= policy.cellMs || bms.lastCellPollMs == 0)) {
      requestCells();
      bms.lastCellPollMs = now;
      return;
    }
    const BmsParser parser = activeProfile().parser;
    if ((parser == BmsParser::Watt || parser == BmsParser::Jk) &&
        (bms.lastProductMs == 0 || now - bms.lastProductPollMs > 60000)) {
      requestProduct();
      bms.lastProductPollMs = now;
      return;
    }
    if (parser == BmsParser::Watt &&
        (now - bms.lastWarningPollMs >= policy.warningMs || bms.lastWarningPollMs == 0)) {
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

  PollPolicy currentPolicy() const {
    PollPolicy policy;
    if (!batteryPowerSaveActive()) return policy;

    if (packCharging()) {
      policy.mode = BlePolicyMode::Charging;
      policy.analogMs = 30000UL;
      policy.cellMs = 60000UL;
      policy.warningMs = 60000UL;
      return policy;
    }

    if (bms.soc >= 98 && dischargeCurrentA() < 0.3f) {
      policy.mode = BlePolicyMode::Idle;
      policy.analogMs = static_cast<uint32_t>(settings.idleBleWakeHours * 3600000.0f);
      policy.cellMs = policy.analogMs;
      policy.warningMs = policy.analogMs;
      policy.keepConnected = false;
      return policy;
    }

    if (workDetected() || socRatePctPerHour > 5.0f || dischargeCurrentA() >= settings.bladesOnAmps) {
      policy.mode = BlePolicyMode::ActiveFast;
      policy.analogMs = 60000UL;
      policy.cellMs = 120000UL;
      policy.warningMs = 120000UL;
      return policy;
    }

    if (activityDetected() || socRatePctPerHour > 2.0f) {
      policy.mode = BlePolicyMode::ActiveMedium;
      policy.analogMs = 180000UL;
      policy.cellMs = 360000UL;
      policy.warningMs = 360000UL;
      policy.keepConnected = false;
      return policy;
    }

    policy.mode = BlePolicyMode::ActiveSlow;
    policy.analogMs = 600000UL;
    policy.cellMs = 1200000UL;
    policy.warningMs = 1200000UL;
    policy.keepConnected = false;
    return policy;
  }

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
    if (profile.parser == BmsParser::Jbd)  return requestJbdRead(JBD_CMD_BASIC_INFO);
    if (profile.parser == BmsParser::Jk)   return requestJkRead(JK_CMD_CELL_INFO);
    if (profile.parser == BmsParser::Daly) return requestDalyStatus();
    return requestWattRead(DP_ANALOG_QUANTITY);
  }

  bool requestCells() {
    const BmsProfile &profile = activeProfile();
    if (profile.parser == BmsParser::Jbd) return requestJbdRead(JBD_CMD_CELL_VOLTAGES);
    return true;
  }

  bool requestProduct() {
    const BmsProfile &profile = activeProfile();
    if (profile.parser == BmsParser::Watt) return requestWattRead(DP_PRODUCT_INFO);
    if (profile.parser == BmsParser::Jk)   return requestJkRead(JK_CMD_DEVICE_INFO);
    return true;
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

  bool requestJkRead(uint8_t command) {
    if (!ready()) return false;
    uint8_t frame[20]{0xAA, 0x55, 0x90, 0xEB, command, 0x00, 0x00, 0x00, 0x00, 0x00,
                      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t crc = 0;
    for (size_t i = 0; i < 19; ++i) crc += frame[i];
    frame[19] = crc;
    const bool ok = writeChar_->writeValue(frame, sizeof(frame), activeProfile().writeWithResponse);
    bms.lastPollMs = millis();
    if (!ok) { bms.errors++; bms.lastError = "BLE write failed"; }
    return ok;
  }

  bool requestDalyStatus() {
    if (!ready()) return false;
    // Read 80 registers from 0x0000 — covers cells (max 32), temps, voltage, SOC, etc.
    uint8_t frame[8]{0xD2, 0x03, 0x00, 0x00, 0x00, 0x50, 0x00, 0x00};
    const uint16_t crc = crc16Modbus(frame, 6);
    frame[6] = static_cast<uint8_t>(crc & 0xFF);
    frame[7] = static_cast<uint8_t>(crc >> 8);
    const bool ok = writeChar_->writeValue(frame, sizeof(frame), activeProfile().writeWithResponse);
    bms.lastPollMs = millis();
    if (!ok) { bms.errors++; bms.lastError = "BLE write failed"; }
    return ok;
  }

  void consumeRxBuffer() {
    const BmsParser parser = activeProfile().parser;
    while (rxLen_ > 0) {
      bool synced = false;
      if      (parser == BmsParser::Jbd  && rxBuffer_[0] == 0xDD) synced = true;
      else if (parser == BmsParser::Watt && rxBuffer_[0] == 0x7E) synced = true;
      else if (parser == BmsParser::Daly && rxBuffer_[0] == 0xD2) synced = true;
      else if (parser == BmsParser::Jk   && rxLen_ >= 4 &&
               rxBuffer_[0] == 0x55 && rxBuffer_[1] == 0xAA &&
               rxBuffer_[2] == 0xEB && rxBuffer_[3] == 0x90) synced = true;
      if (!synced) {
        memmove(rxBuffer_, rxBuffer_ + 1, rxLen_ - 1);
        rxLen_--;
        bms.errors++;
        continue;
      }
      size_t expected = 0;
      if      (parser == BmsParser::Jbd)  expected = expectedJbdLength();
      else if (parser == BmsParser::Watt) expected = expectedWattLength();
      else if (parser == BmsParser::Daly) expected = expectedDalyLength();
      else if (parser == BmsParser::Jk)   expected = expectedJkLength();
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

  size_t expectedDalyLength() const {
    if (rxLen_ < 3 || rxBuffer_[0] != 0xD2 || rxBuffer_[1] != 0x03) return 0;
    const uint8_t dataLen = rxBuffer_[2];
    if (dataLen < 10) return 0;
    return static_cast<size_t>(dataLen) + 5;  // D2 03 len [data] crc_lo crc_hi
  }

  size_t expectedJkLength() const {
    // JK CELL_INFO = 300 bytes, DEVICE_INFO = 48 bytes (both fixed per command)
    if (rxLen_ < 5) return 0;
    const uint8_t cmd = rxBuffer_[4];
    if (cmd == JK_CMD_CELL_INFO)   return 300;
    if (cmd == JK_CMD_DEVICE_INFO) return 48;
    return 0;
  }

  void parseFrame(const uint8_t *frame, size_t len) {
    switch (activeProfile().parser) {
      case BmsParser::Jbd:  parseJbdFrame(frame, len);  return;
      case BmsParser::Jk:   parseJkFrame(frame, len);   return;
      case BmsParser::Daly: parseDalyFrame(frame, len); return;
      default:              parseWattFrame(frame, len);  return;
    }
  }

  void parseWattFrame(const uint8_t *frame, size_t len) {
    const uint16_t dataLen = readU16(frame + 6);
    if (len < 11 || frame[len - 1] != 0x0D || static_cast<size_t>(dataLen) + 11 != len) {
      bms.errors++;
      bms.lastError = "invalid BLE frame";
      return;
    }
    const uint16_t gotCrcBe = readU16(frame + len - 3);
    const uint16_t gotCrcLe = readU16LE(frame + len - 3);
    // Try CRC over full frame and over frame[1..] (some BMS implementations exclude the 7E start byte)
    const uint16_t wantCrc1 = crc16Modbus(frame, len - 3);
    const uint16_t wantCrc2 = len >= 4 ? crc16Modbus(frame + 1, len - 4) : 0;
    if (gotCrcBe != wantCrc1 && gotCrcLe != wantCrc1 &&
        gotCrcBe != wantCrc2 && gotCrcLe != wantCrc2) {
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

  // ── JK BMS (Jikong) ──────────────────────────────────────────────────────
  // Protocol: FFE0/FFE1, 20-byte request AA 55 90 EB [cmd] 00..CRC
  // Response: 55 AA EB 90 [cmd] [seq] [payload…] [crc_sum]
  // Offsets below are absolute from frame[0]=0x55.
  // CELL_INFO (0x96): 300 bytes total. DEVICE_INFO (0x97): 48 bytes total.

  void parseJkFrame(const uint8_t *frame, size_t len) {
    if (len < 6 || frame[0] != 0x55 || frame[1] != 0xAA || frame[2] != 0xEB || frame[3] != 0x90) {
      bms.errors++; bms.lastError = "invalid JK frame"; return;
    }
    uint8_t crc = 0;
    for (size_t i = 0; i < len - 1; ++i) crc += frame[i];
    if (crc != frame[len - 1]) {
      bms.errors++; bms.lastError = "JK CRC mismatch"; return;
    }
    bms.lastError = "";
    bms.frames++;
    bms.latestFrameHex = bytesToHex(frame, len, 96);
    bms.status = "data received";
    const uint8_t cmd = frame[4];
    if      (cmd == JK_CMD_CELL_INFO)   parseJkCellInfo(frame, len);
    else if (cmd == JK_CMD_DEVICE_INFO) parseJkDeviceInfo(frame, len);
  }

  void parseJkCellInfo(const uint8_t *frame, size_t len) {
    // Cell voltages: frame[6..53] = 24 × uint16 LE × 0.001 V
    if (len < 56) return;
    bms.minCellV = 0.0f;
    bms.maxCellV = 0.0f;
    uint8_t cells = 0;
    for (uint8_t i = 0; i < 24; ++i) {
      const float v = readU16LE(frame + 6 + i * 2) * 0.001f;
      if (v > 0.5f) cells = i + 1;
    }
    bms.cellCount = cells;
    for (uint8_t i = 0; i < cells; ++i) {
      const float v = readU16LE(frame + 6 + i * 2) * 0.001f;
      bms.cellVolts[i] = v;
      if (i == 0 || v < bms.minCellV) bms.minCellV = v;
      if (i == 0 || v > bms.maxCellV) bms.maxCellV = v;
    }
    bms.deltaCellMv = (bms.maxCellV - bms.minCellV) * 1000.0f;
    if (len < 155) { bms.lastAnalogMs = millis(); return; }
    // frame[118..121]: total pack voltage, uint32 LE, ×0.001 V
    bms.packVoltage  = readU32LE(frame + 118) * 0.001f;
    // frame[126..129]: current, int32 LE, ×0.001 A (sign: positive=charge on most JK FW)
    bms.currentA     = static_cast<float>(static_cast<int32_t>(readU32LE(frame + 126))) * 0.001f;
    // frame[130..131]: probe T1 (×0.1 °C), frame[132..133]: probe T2, frame[134..135]: MOS
    bms.pcbTempC     = static_cast<float>(static_cast<int16_t>(readU16LE(frame + 130))) * 0.1f;
    bms.mosTempC     = static_cast<float>(static_cast<int16_t>(readU16LE(frame + 134))) * 0.1f;
    bms.tempsC[0]    = static_cast<float>(static_cast<int16_t>(readU16LE(frame + 132))) * 0.1f;
    bms.tempCount    = 1;
    // frame[140]: balance state, frame[141]: SOC %, frame[142..145]: remaining Ah, frame[146..149]: full Ah
    bms.balanceState = frame[140];
    bms.soc          = frame[141];
    bms.remainingAh  = readU32LE(frame + 142) * 0.001f;
    bms.totalAh      = readU32LE(frame + 146) * 0.001f;
    bms.designAh     = bms.totalAh;
    // frame[150..153]: cycle count
    if (len > 154) bms.cycleCount = static_cast<uint16_t>(readU32LE(frame + 150));
    bms.lastAnalogMs = millis();
  }

  void parseJkDeviceInfo(const uint8_t *frame, size_t len) {
    if (len < 48) return;
    bms.software     = paddedAscii(frame, 6, 16);
    bms.manufacturer = paddedAscii(frame, 22, 16);
    bms.serial       = paddedAscii(frame, 38, 10);
    bms.lastProductMs = millis();
  }

  // ── Daly Smart BMS ───────────────────────────────────────────────────────
  // Protocol: FFF0/FFF1/FFF2 (no auth), D2 Modbus-style
  // Request:  D2 03 [addr_hi] [addr_lo] [cnt_hi] [cnt_lo] [crc_lo] [crc_hi]
  // Response: D2 03 [len] [data...] [crc_lo] [crc_hi]
  // All offsets below are absolute from frame[0]=0xD2.

  void parseDalyFrame(const uint8_t *frame, size_t len) {
    if (len < 5 || frame[0] != 0xD2 || frame[1] != 0x03) {
      bms.errors++; bms.lastError = "invalid Daly frame"; return;
    }
    const uint8_t dataLen = frame[2];
    if (static_cast<size_t>(dataLen) + 5 != len) {
      bms.errors++; bms.lastError = "invalid Daly length"; return;
    }
    const uint16_t gotCrc  = static_cast<uint16_t>(frame[3 + dataLen]) |
                             (static_cast<uint16_t>(frame[4 + dataLen]) << 8);
    const uint16_t calcCrc = crc16Modbus(frame, 3 + dataLen);
    if (gotCrc != calcCrc) {
      bms.errors++; bms.lastError = "Daly CRC mismatch"; return;
    }
    bms.lastError = "";
    bms.frames++;
    bms.latestFrameHex = bytesToHex(frame, len, 96);
    bms.status = "data received";
    parseDalyStatus(frame, len);
  }

  void parseDalyStatus(const uint8_t *frame, size_t len) {
    // Minimum for 62-register response: 129 bytes; for 80-register: 165 bytes
    if (len < 69) return;
    // Cell count at frame[102] (low byte of uint16 BE at 101-102)
    const uint8_t cells = min(frame[102], static_cast<uint8_t>(32));
    bms.cellCount = cells;
    bms.minCellV  = 0.0f;
    bms.maxCellV  = 0.0f;
    // Cell voltages: frame[3 + i*2], BE, ×0.001 V
    for (uint8_t i = 0; i < cells && (3 + i * 2 + 1) < len; ++i) {
      const float v = readU16(frame + 3 + i * 2) * 0.001f;
      bms.cellVolts[i] = v;
      if (i == 0 || v < bms.minCellV) bms.minCellV = v;
      if (i == 0 || v > bms.maxCellV) bms.maxCellV = v;
    }
    bms.deltaCellMv = (bms.maxCellV - bms.minCellV) * 1000.0f;
    if (len < 90) { bms.lastAnalogMs = millis(); return; }
    // Temperatures: frame[67 + i*2], BE, (raw-40)°C. Count at frame[104].
    const uint8_t tempCount = min(frame[104], static_cast<uint8_t>(8));
    bms.tempCount = tempCount;
    for (uint8_t i = 0; i < tempCount && (67 + i * 2 + 1) < len; ++i) {
      const float t = static_cast<float>(readU16(frame + 67 + i * 2)) - 40.0f;
      if      (i == 0) bms.mosTempC = t;
      else if (i == 1) bms.pcbTempC = t;
      else             bms.tempsC[i - 2] = t;
    }
    // frame[83..84]: total voltage BE ×0.1V; frame[85..86]: current (raw-30000)×0.1A; frame[87..88]: SOC ×0.1%
    bms.packVoltage = readU16(frame + 83) * 0.1f;
    bms.currentA    = (static_cast<float>(readU16(frame + 85)) - 30000.0f) * 0.1f;
    bms.soc         = static_cast<uint16_t>(readU16(frame + 87) / 10);
    if (len < 110) { bms.lastAnalogMs = millis(); return; }
    // frame[99..100]: remaining Ah ×0.1; frame[105..106]: cycles; frame[107..108]: balance on/off
    bms.remainingAh  = readU16(frame + 99) * 0.1f;
    bms.totalAh      = bms.remainingAh * 100.0f / max(1.0f, static_cast<float>(bms.soc));
    bms.designAh     = bms.totalAh;
    bms.cycleCount   = readU16(frame + 105);
    bms.balanceState = readU16(frame + 107) != 0 ? 1 : 0;
    if (len > 116) bms.deltaCellMv = readU16(frame + 115) * 1.0f;  // ×0.001V×1000 = mV
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

  static uint16_t readU16LE(const uint8_t *data) {
    return static_cast<uint16_t>(data[0]) | (static_cast<uint16_t>(data[1]) << 8);
  }

  static uint32_t readU32LE(const uint8_t *data) {
    return static_cast<uint32_t>(data[0]) | (static_cast<uint32_t>(data[1]) << 8) |
           (static_cast<uint32_t>(data[2]) << 16) | (static_cast<uint32_t>(data[3]) << 24);
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
  uint8_t rxBuffer_[512]{};
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

// ── MQTT helpers ─────────────────────────────────────────────────────────────

String mqttEffectivePrefix() {
  return settings.mqttTopicPrefix.isEmpty()
    ? String("r48display/") + settings.hostname
    : settings.mqttTopicPrefix;
}

void setupMqtt() {
  if (!settings.mqttEnabled || settings.mqttHost.isEmpty()) return;
  mqttClient.begin(settings.mqttHost, settings.mqttPort,
                   settings.mqttUser, settings.mqttPassword,
                   mqttEffectivePrefix(),
                   String("r48display-") + chipSuffix());
}

void publishMqttState(bool heartbeat = false) {
  if (!mqttClient.connected()) return;
  JsonDocument doc;
  doc["soc"] = (int)bms.soc;
  doc["voltage"] = serialized(String(bms.packVoltage, 2));
  const float signedA = chargeCurrentA() - dischargeCurrentA();
  doc["current"] = serialized(String(signedA, 2));
  doc["power"] = static_cast<int>((chargeCurrentA() - dischargeCurrentA()) * bms.packVoltage);
  doc["remaining_ah"] = serialized(String(bms.remainingAh * capacityScale(), 1));
  doc["cycles"] = bms.cycleCount;
  doc["cell_delta_mv"] = static_cast<int>(bms.deltaCellMv + 0.5f);
  doc["mos_temp_c"] = serialized(String(bms.mosTempC, 1));
  doc["health_pct"] = serialized(String(bmsHealthPercent(), 1));
  const ActivityState as = activityState();
  doc["status"] = activityStateKey(as);
  doc["status_label"] = stateLabel(as);
  doc["hours_total"] = serialized(String(settings.hoursBaseline + hoursTotal, 1));
  doc["hours_active"] = serialized(String(hoursActive, 1));
  doc["hours_working"] = serialized(String(hoursWorking, 1));
  doc["link"] = heartbeat
    ? String(bms.connected ? "stale" : "disconnected")
    : bmsLinkLabel();
  String payload; serializeJson(doc, payload);
  mqttClient.publish("state", payload);
}

void publishMqttCells() {
  if (!mqttClient.connected() || bms.cellCount == 0) return;
  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();
  for (uint8_t i = 0; i < bms.cellCount && i < 32; i++)
    arr.add(serialized(String(bms.cellVolts[i], 3)));
  String payload; serializeJson(doc, payload);
  mqttClient.publish("cells", payload);
}

void publishMqttDiscovery() {
  if (!mqttClient.connected()) return;
  const String chipId = chipSuffix();
  const String devId = "r48display_" + chipId;
  const String stateTopic = mqttClient.prefix() + "/state";
  const String availTopic = mqttClient.prefix() + "/availability";
  const String devName = String("R48Display (") + settings.hostname + ")";
  const char *tempUnit = settings.tempUnit == "F" ? "°F" : "°C";

  auto buildDev = [&](JsonDocument &d) {
    JsonObject dev = d["device"].to<JsonObject>();
    JsonArray ids = dev["identifiers"].to<JsonArray>();
    ids.add(devId);
    dev["name"] = devName;
    dev["model"] = "R48Display";
    dev["manufacturer"] = "R48Display";
    dev["sw_version"] = FIRMWARE_VERSION;
  };

  auto sensor = [&](const char *eid, const char *name, const char *vTpl,
                    const char *unit, const char *devClass, const char *icon) {
    JsonDocument d;
    d["name"] = name;
    d["unique_id"] = devId + "_" + eid;
    d["state_topic"] = stateTopic;
    d["value_template"] = vTpl;
    if (unit && *unit) d["unit_of_measurement"] = unit;
    if (devClass && *devClass) d["device_class"] = devClass;
    if (icon && *icon) d["icon"] = icon;
    d["availability_topic"] = availTopic;
    buildDev(d);
    String p; serializeJson(d, p);
    mqttClient.publishRaw((String("homeassistant/sensor/") + devId + "/" + eid + "/config").c_str(), p, true);
  };

  auto bsensor = [&](const char *eid, const char *name, const char *vTpl, const char *icon) {
    JsonDocument d;
    d["name"] = name;
    d["unique_id"] = devId + "_" + eid;
    d["state_topic"] = stateTopic;
    d["value_template"] = vTpl;
    if (icon && *icon) d["icon"] = icon;
    d["availability_topic"] = availTopic;
    buildDev(d);
    String p; serializeJson(d, p);
    mqttClient.publishRaw((String("homeassistant/binary_sensor/") + devId + "/" + eid + "/config").c_str(), p, true);
  };

  sensor("soc",         "Battery SOC",       "{{ value_json.soc }}",           "%",  "battery",     "mdi:battery");
  sensor("voltage",     "Pack Voltage",       "{{ value_json.voltage }}",        "V",  "voltage",     "mdi:flash");
  sensor("current",     "Pack Current",       "{{ value_json.current }}",        "A",  "current",     "mdi:current-dc");
  sensor("power",       "Pack Power",         "{{ value_json.power }}",          "W",  "power",       "mdi:lightning-bolt");
  sensor("remaining_ah","Remaining Ah",       "{{ value_json.remaining_ah }}",   "Ah", "",            "mdi:battery-charging");
  sensor("cycles",      "Cycle Count",        "{{ value_json.cycles }}",         "",   "",            "mdi:counter");
  sensor("cell_delta",  "Cell Spread",        "{{ value_json.cell_delta_mv }}",  "mV", "",            "mdi:battery-alert");
  sensor("mos_temp",    "MOS Temperature",    "{{ value_json.mos_temp_c }}",     tempUnit, "temperature", "mdi:thermometer");
  sensor("status",      "Activity Status",    "{{ value_json.status }}",         "",   "",            "mdi:information");
  sensor("ble_link",    "BLE Link",           "{{ value_json.link }}",           "",   "",            "mdi:bluetooth");
  sensor("health",      "Pack Health",        "{{ value_json.health_pct }}",     "%",  "",            "mdi:heart-pulse");
  sensor("hours_total", "Total Hours",        "{{ value_json.hours_total }}",    "h",  "duration",    "mdi:clock-outline");
  sensor("hours_active","Active Hours",       "{{ value_json.hours_active }}",   "h",  "duration",    "mdi:clock-fast");
  sensor("hours_work",  "Working Hours",      "{{ value_json.hours_working }}",  "h",  "duration",    "mdi:clock-alert");
  bsensor("is_charging","Is Charging",
    "{% if value_json.status == 'charging' %}ON{% else %}OFF{% endif %}", "mdi:battery-charging");
}

void updateMqtt() {
  if (!settings.mqttEnabled) return;
  mqttClient.loop();
  if (!mqttClient.connected()) return;

  // One-time discovery on new connection
  if (mqttClient.newConnection()) {
    publishMqttDiscovery();
    publishMqttState();
  }

  // Publish on new BMS analog data (M6-06)
  static uint32_t mqttLastAnalogMs = 0;
  if (bms.lastAnalogMs > 0 && bms.lastAnalogMs != mqttLastAnalogMs) {
    mqttLastAnalogMs = bms.lastAnalogMs;
    publishMqttState();
    publishMqttCells();
  }

  // Publish on activity state change (M6-07)
  static const char *prevStateKey = "";
  const char *curKey = activityStateKey(activityState());
  if (curKey != prevStateKey) { prevStateKey = curKey; publishMqttState(); }

  // Heartbeat every 60 s when BMS is not providing fresh data (M6-08)
  static uint32_t hbMs = 0;
  if (millis() - hbMs >= 60000UL && (mqttLastAnalogMs == 0 ||
      millis() - bms.lastAnalogMs > 30000UL)) {
    hbMs = millis();
    publishMqttState(true);
  }
}

void apiMqttPublishNow() {
  if (!settings.mqttEnabled || !mqttClient.connected()) {
    server.send(503, "application/json", "{\"ok\":false,\"error\":\"MQTT not connected\"}");
    return;
  }
  publishMqttState();
  publishMqttCells();
  server.send(200, "application/json", "{\"ok\":true}");
}

void apiMqttRediscover() {
  if (!settings.mqttEnabled || !mqttClient.connected()) {
    server.send(503, "application/json", "{\"ok\":false,\"error\":\"MQTT not connected\"}");
    return;
  }
  publishMqttDiscovery();
  server.send(200, "application/json", "{\"ok\":true}");
}

void apiMqttTest() {
  if (!settings.mqttEnabled || settings.mqttHost.isEmpty()) {
    server.send(400, "application/json", "{\"ok\":false,\"error\":\"MQTT not configured\"}");
    return;
  }
  JsonDocument doc;
  doc["ok"] = mqttClient.connected();
  doc["status"] = mqttClient.statusLabel();
  doc["broker"] = settings.mqttHost + ":" + String(settings.mqttPort);
  sendJson(doc);
}

// ── end MQTT helpers ──────────────────────────────────────────────────────────

// ── Battery degradation tracking ─────────────────────────────────────────────

void loadDegradation() {
  prefs.begin("r48disp", true);
  degradation.maxCellSpreadMv = prefs.getFloat("dMaxSpd", 0.0f);
  degradation.minCellVoltageV = prefs.getFloat("dMinCv", 9.9f);
  degradation.maxTempC = prefs.getFloat("dMaxTc", -99.0f);
  degradation.lowVoltageEvents = prefs.getUInt("dLvEvt", 0);
  degradation.highCurrentEvents = prefs.getUInt("dHcEvt", 0);
  prefs.end();
}

void saveDegradation() {
  prefs.begin("r48disp", false);
  prefs.putFloat("dMaxSpd", degradation.maxCellSpreadMv);
  prefs.putFloat("dMinCv", degradation.minCellVoltageV);
  prefs.putFloat("dMaxTc", degradation.maxTempC);
  prefs.putUInt("dLvEvt", degradation.lowVoltageEvents);
  prefs.putUInt("dHcEvt", degradation.highCurrentEvents);
  prefs.end();
}

void updateDegradation() {
  if (!bms.found || bms.lastAnalogMs == 0) return;
  static uint32_t lastDegradMs = 0;
  if (lastDegradMs == bms.lastAnalogMs) return;
  lastDegradMs = bms.lastAnalogMs;

  bool dirty = false;
  if (bms.deltaCellMv > degradation.maxCellSpreadMv) {
    degradation.maxCellSpreadMv = bms.deltaCellMv; dirty = true;
  }
  if (bms.minCellV > 0.1f && bms.minCellV < degradation.minCellVoltageV) {
    degradation.minCellVoltageV = bms.minCellV; dirty = true;
  }
  const float maxTemp = max(bms.mosTempC, bms.pcbTempC);
  if (maxTemp > 0.0f && maxTemp > degradation.maxTempC) {
    degradation.maxTempC = maxTemp; dirty = true;
  }
  // Low-voltage event: any cell under floor during discharge (deduplicated per discharge cycle)
  static bool lvThisDischarge = false;
  if (chargeCurrentA() > 0.5f) lvThisDischarge = false;  // reset on charge
  if (bms.minCellV > 0.1f && bms.minCellV < settings.lowVoltageFloorV && dischargeCurrentA() > 0.1f && !lvThisDischarge) {
    lvThisDischarge = true;
    degradation.lowVoltageEvents++;
    dirty = true;
  }
  // High-current event: sustained discharge above work threshold for 5 s
  static uint32_t hcStartMs = 0;
  static bool hcCounted = false;
  const float workA = max(settings.bladesOnAmps, settings.mowerRunAmps);
  if (dischargeCurrentA() > workA && workA > 1.0f) {
    if (hcStartMs == 0) { hcStartMs = millis(); hcCounted = false; }
    else if (!hcCounted && millis() - hcStartMs >= 5000UL) {
      hcCounted = true;
      degradation.highCurrentEvents++;
      dirty = true;
    }
  } else {
    hcStartMs = 0; hcCounted = false;
  }
  if (dirty) saveDegradation();
}

// ── end degradation ───────────────────────────────────────────────────────────

// ── Maintenance tracker ──────────────────────────────────────────────────────

const char *maintTypeName(MaintType t) {
  switch (t) {
    case MaintType::HoursTotal:   return "HOURS_TOTAL";
    case MaintType::HoursWorking: return "HOURS_WORKING";
    case MaintType::Days:         return "DAYS";
    default:                      return "HOURS_ACTIVE";
  }
}

MaintType maintTypeFromStr(const char *s) {
  if (strcmp(s, "HOURS_TOTAL") == 0)   return MaintType::HoursTotal;
  if (strcmp(s, "HOURS_WORKING") == 0) return MaintType::HoursWorking;
  if (strcmp(s, "DAYS") == 0)          return MaintType::Days;
  return MaintType::HoursActive;
}

uint8_t nextMaintId() {
  for (uint8_t id = 1; id <= 20; id++) {
    bool taken = false;
    for (const auto &it : maintenanceItems) if (it.id == id) { taken = true; break; }
    if (!taken) return id;
  }
  return 0;
}

void loadMaintenance() {
  prefs.begin("r48disp", true);
  const String json = prefs.getString("maint", "[]");
  prefs.end();
  JsonDocument doc;
  if (deserializeJson(doc, json) != DeserializationError::Ok) return;
  maintenanceItems.clear();
  for (JsonObject obj : doc.as<JsonArray>()) {
    MaintenanceItem item;
    item.id = obj["id"] | 0;
    item.name = String(obj["name"] | "");
    item.type = maintTypeFromStr(obj["type"] | "HOURS_ACTIVE");
    item.interval = obj["interval"] | 100.0f;
    item.lastResetTs = obj["last_reset_ts"] | (uint32_t)0;
    item.lastResetVal = obj["last_reset_val"] | 0.0f;
    item.notes = String(obj["notes"] | "");
    if (item.id > 0 && item.id <= 20 && item.name.length() > 0)
      maintenanceItems.push_back(item);
  }
}

void saveMaintenance() {
  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();
  for (const auto &item : maintenanceItems) {
    JsonObject obj = arr.add<JsonObject>();
    obj["id"] = item.id;
    obj["name"] = item.name;
    obj["type"] = maintTypeName(item.type);
    obj["interval"] = serialized(String(item.interval, 2));
    obj["last_reset_ts"] = item.lastResetTs;
    obj["last_reset_val"] = serialized(String(item.lastResetVal, 2));
    obj["notes"] = item.notes;
  }
  String out;
  serializeJson(doc, out);
  prefs.begin("r48disp", false);
  prefs.putString("maint", out);
  prefs.end();
}

void initDefaultMaintenanceItems() {
  if (!maintenanceItems.empty()) return;
  const struct { const char *name; MaintType type; float interval; const char *notes; } defaults[] = {
    {"Check blades / cutting edges", MaintType::HoursWorking, 25.0f, ""},
    {"Inspect tires / wheels",       MaintType::HoursActive,  100.0f, ""},
    {"Annual inspection",             MaintType::Days,         365.0f, ""},
  };
  for (const auto &d : defaults) {
    MaintenanceItem item;
    item.id = nextMaintId();
    item.name = d.name;
    item.type = d.type;
    item.interval = d.interval;
    item.notes = d.notes;
    maintenanceItems.push_back(item);
  }
  saveMaintenance();
}

void loadMaintenanceHistoryFor(uint8_t id) {
  if (maintHistory.count(id)) return;
  char key[8];
  snprintf(key, sizeof(key), "mh_%u", id);
  prefs.begin("r48disp", true);
  const String json = prefs.getString(key, "[]");
  prefs.end();
  JsonDocument doc;
  auto &entries = maintHistory[id];
  entries.clear();
  if (deserializeJson(doc, json) != DeserializationError::Ok) return;
  for (JsonObject obj : doc.as<JsonArray>()) {
    MaintenanceHistoryEntry e;
    e.ts    = obj["ts"]    | (uint32_t)0;
    e.val   = obj["val"]   | 0.0f;
    e.notes = String(obj["notes"] | "");
    if (e.ts > 0) entries.push_back(e);
  }
}

void saveMaintenanceHistoryFor(uint8_t id) {
  auto it = maintHistory.find(id);
  if (it == maintHistory.end()) return;
  char key[8];
  snprintf(key, sizeof(key), "mh_%u", id);
  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();
  for (const auto &e : it->second) {
    JsonObject obj = arr.add<JsonObject>();
    obj["ts"]    = e.ts;
    obj["val"]   = serialized(String(e.val, 2));
    obj["notes"] = e.notes;
  }
  String out;
  serializeJson(doc, out);
  prefs.begin("r48disp", false);
  prefs.putString(key, out);
  prefs.end();
}

void clearMaintenanceHistoryFor(uint8_t id) {
  maintHistory.erase(id);
  char key[8];
  snprintf(key, sizeof(key), "mh_%u", id);
  prefs.begin("r48disp", false);
  prefs.remove(key);
  prefs.end();
}

float maintCurrentValue(const MaintenanceItem &item) {
  switch (item.type) {
    case MaintType::HoursTotal:   return settings.hoursBaseline + hoursTotal;
    case MaintType::HoursWorking: return hoursWorking;
    case MaintType::Days: {
      const time_t now = time(nullptr);
      return (now > 0 && item.lastResetTs > 0)
        ? static_cast<float>(now - item.lastResetTs) / 86400.0f : 0.0f;
    }
    default: return hoursActive;
  }
}

float maintElapsed(const MaintenanceItem &item) {
  if (item.type == MaintType::Days) return maintCurrentValue(item);
  return max(0.0f, maintCurrentValue(item) - item.lastResetVal);
}

bool maintOverdue(const MaintenanceItem &item) {
  return item.interval > 0.0f && maintElapsed(item) >= item.interval;
}

uint8_t maintOverdueCount() {
  uint8_t n = 0;
  for (const auto &it : maintenanceItems) if (maintOverdue(it)) n++;
  return n;
}

void addMaintJsonItem(JsonObject &obj, const MaintenanceItem &item) {
  obj["id"] = item.id;
  obj["name"] = item.name;
  obj["type"] = maintTypeName(item.type);
  obj["interval"] = serialized(String(item.interval, 2));
  obj["last_reset_ts"] = item.lastResetTs;
  obj["last_reset_val"] = serialized(String(item.lastResetVal, 2));
  obj["notes"] = item.notes;
  const float elapsed = maintElapsed(item);
  const float remaining = max(0.0f, item.interval - elapsed);
  const float pct = item.interval > 0.0f ? min(100.0f, elapsed / item.interval * 100.0f) : 0.0f;
  obj["elapsed"] = serialized(String(elapsed, 2));
  obj["remaining"] = serialized(String(remaining, 2));
  obj["pct"] = serialized(String(pct, 1));
  obj["overdue"] = maintOverdue(item);
}

void apiMaintenanceGet() {
  JsonDocument doc;
  JsonArray arr = doc.to<JsonArray>();
  for (const auto &item : maintenanceItems) {
    JsonObject obj = arr.add<JsonObject>();
    addMaintJsonItem(obj, item);
  }
  sendJson(doc);
}

void apiMaintenancePost() {
  const String name = server.arg("name");
  if (name.isEmpty()) { server.send(400, "text/plain", "name required"); return; }
  const String typeStr = server.arg("type");
  const float interval = server.arg("interval").toFloat();
  if (interval <= 0.0f) { server.send(400, "text/plain", "interval must be > 0"); return; }
  const uint8_t id = static_cast<uint8_t>(server.arg("id").toInt());
  MaintenanceItem *existing = nullptr;
  for (auto &it : maintenanceItems) if (it.id == id) { existing = &it; break; }
  if (existing) {
    existing->name = name;
    existing->type = maintTypeFromStr(typeStr.c_str());
    existing->interval = interval;
    existing->notes = server.arg("notes");
  } else {
    if (maintenanceItems.size() >= 20) { server.send(400, "text/plain", "max 20 items"); return; }
    MaintenanceItem item;
    item.id = nextMaintId();
    if (!item.id) { server.send(500, "text/plain", "no id available"); return; }
    item.name = name;
    item.type = maintTypeFromStr(typeStr.c_str());
    item.interval = interval;
    item.notes = server.arg("notes");
    maintenanceItems.push_back(item);
    existing = &maintenanceItems.back();
  }
  saveMaintenance();
  JsonDocument doc;
  JsonObject obj = doc.to<JsonObject>();
  addMaintJsonItem(obj, *existing);
  sendJson(doc);
}

void apiMaintenanceConfirm() {
  const uint8_t id = static_cast<uint8_t>(server.arg("id").toInt());
  MaintenanceItem *item = nullptr;
  for (auto &it : maintenanceItems) if (it.id == id) { item = &it; break; }
  if (!item) { server.send(404, "text/plain", "item not found"); return; }
  const float val = maintCurrentValue(*item);
  const uint32_t now = static_cast<uint32_t>(time(nullptr));
  loadMaintenanceHistoryFor(id);
  auto &hist = maintHistory[id];
  MaintenanceHistoryEntry entry;
  entry.ts    = now;
  entry.val   = val;
  entry.notes = server.arg("notes");
  hist.push_back(entry);
  if (hist.size() > MAINT_HISTORY_MAX)
    hist.erase(hist.begin(), hist.begin() + (hist.size() - MAINT_HISTORY_MAX));
  saveMaintenanceHistoryFor(id);
  item->lastResetTs  = now;
  item->lastResetVal = val;
  saveMaintenance();
  JsonDocument doc;
  JsonObject obj = doc.to<JsonObject>();
  addMaintJsonItem(obj, *item);
  sendJson(doc);
}

void apiMaintenanceDelete() {
  const uint8_t id = static_cast<uint8_t>(server.arg("id").toInt());
  const auto before = maintenanceItems.size();
  maintenanceItems.erase(
    std::remove_if(maintenanceItems.begin(), maintenanceItems.end(),
      [id](const MaintenanceItem &it){ return it.id == id; }),
    maintenanceItems.end());
  if (maintenanceItems.size() == before) { server.send(404, "text/plain", "not found"); return; }
  clearMaintenanceHistoryFor(id);
  saveMaintenance();
  server.send(200, "application/json", "{\"ok\":true}");
}

void apiMaintenanceHistoryGet() {
  const uint8_t id = static_cast<uint8_t>(server.arg("id").toInt());
  const MaintenanceItem *item = nullptr;
  for (const auto &it : maintenanceItems) if (it.id == id) { item = &it; break; }
  if (!item) { server.send(404, "text/plain", "item not found"); return; }
  loadMaintenanceHistoryFor(id);
  JsonDocument doc;
  JsonObject root = doc.to<JsonObject>();
  root["type"] = maintTypeName(item->type);
  JsonArray arr = root["entries"].to<JsonArray>();
  for (const auto &e : maintHistory[id]) {
    JsonObject obj = arr.add<JsonObject>();
    obj["ts"]    = e.ts;
    obj["val"]   = serialized(String(e.val, 2));
    obj["notes"] = e.notes;
  }
  sendJson(doc);
}

void apiMaintenanceHistoryDelete() {
  const uint8_t id = static_cast<uint8_t>(server.arg("id").toInt());
  const uint32_t ts = static_cast<uint32_t>(server.arg("ts").toInt());
  bool found = false;
  for (const auto &it : maintenanceItems) if (it.id == id) { found = true; break; }
  if (!found) { server.send(404, "text/plain", "item not found"); return; }
  loadMaintenanceHistoryFor(id);
  auto &hist = maintHistory[id];
  const auto before = hist.size();
  hist.erase(std::remove_if(hist.begin(), hist.end(),
    [ts](const MaintenanceHistoryEntry &e){ return e.ts == ts; }), hist.end());
  if (hist.size() == before) { server.send(404, "text/plain", "entry not found"); return; }
  saveMaintenanceHistoryFor(id);
  server.send(200, "application/json", "{\"ok\":true}");
}

static String csvEscape(const String &s) {
  if (s.indexOf(',') < 0 && s.indexOf('"') < 0 && s.indexOf('\n') < 0) return s;
  String out = "\"";
  for (char c : s) { if (c == '"') out += '"'; out += c; }
  out += '"';
  return out;
}

void apiMaintenanceExport() {
  String csv = F("Item,Type,Interval,Completed At,Value At Completion,Notes\n");
  for (const auto &item : maintenanceItems) {
    loadMaintenanceHistoryFor(item.id);
    const auto hit = maintHistory.find(item.id);
    if (hit == maintHistory.end() || hit->second.empty()) {
      csv += csvEscape(item.name) + ',' + maintTypeName(item.type) + ',' +
             String(item.interval, 1) + ",,,\n";
    } else {
      for (const auto &e : hit->second) {
        char buf[32] = "unknown";
        if (e.ts > 0) {
          time_t t = static_cast<time_t>(e.ts);
          struct tm tm;
          if (localtime_r(&t, &tm)) strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
        }
        csv += csvEscape(item.name) + ',' + maintTypeName(item.type) + ',' +
               String(item.interval, 1) + ',' + buf + ',' +
               String(e.val, 2) + ',' + csvEscape(e.notes) + '\n';
      }
    }
  }
  server.sendHeader("Content-Disposition", "attachment; filename=\"maintenance.csv\"");
  server.send(200, "text/csv", csv);
}

// ── end Maintenance tracker ──────────────────────────────────────────────────

void loadSettings() {
  prefs.begin("r48disp", true);
  settings.hostname = prefs.getString("host", defaultHostname());
  settings.title = prefs.getString("title", settings.title);
  settings.tempUnit = prefs.getString("tempUnit", settings.tempUnit);
  settings.timeFormat = prefs.getString("timeFmt", settings.timeFormat);
  settings.apPassword = prefs.getString("apPass", settings.apPassword);
  settings.otaPassword = prefs.getString("otaPass", settings.otaPassword);
  settings.wifiSsid = prefs.getString("ssid", "");
  settings.wifiPassword = prefs.getString("wifiPass", "");
  settings.bmsName = prefs.getString("bmsName", "");
  settings.bmsProtocol = prefs.getString("bmsProto", settings.bmsProtocol);
  settings.mowerModel = prefs.getString("model", settings.mowerModel);
  settings.subtitle = prefs.getString("subtitle", settings.subtitle);
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
  settings.timezone = prefs.getString("tz", settings.timezone);
  settings.brightness = prefs.getUChar("bright", 210);
  settings.displayRotation = normalizeDisplayRotation(prefs.getUShort("rotation", 0));
  settings.lcdTimeoutSec = prefs.getUShort("lcdTo", settings.lcdTimeoutSec);
  settings.powerSaveEnabled = prefs.getBool("pwrSave", settings.powerSaveEnabled);
  settings.idleBleWakeHours = prefs.getFloat("idleBleH", settings.idleBleWakeHours);
  settings.lowVoltageFloorV = prefs.getFloat("lvFloorV", settings.lowVoltageFloorV);
  settings.boardBatteryLowPct = prefs.getUChar("bbLowPct", settings.boardBatteryLowPct);
  settings.mqttEnabled = prefs.getBool("mqttEn", settings.mqttEnabled);
  settings.mqttHost = prefs.getString("mqttHost", settings.mqttHost);
  settings.mqttPort = static_cast<uint16_t>(prefs.getUInt("mqttPort", settings.mqttPort));
  settings.mqttUser = prefs.getString("mqttUser", settings.mqttUser);
  settings.mqttPassword = prefs.getString("mqttPass", settings.mqttPassword);
  settings.mqttTopicPrefix = prefs.getString("mqttPfx", settings.mqttTopicPrefix);
  settings.featureMic = prefs.getBool("mic", settings.featureMic);
  settings.micRunThreshold = prefs.getFloat("micTh", settings.micRunThreshold);
  settings.chargeMinAmps = prefs.getFloat("chgMinA", 0.5f);
  settings.ntpEnabled = prefs.getBool("ntpOn", true);
  settings.ntpServer = prefs.getString("ntpHost", DEFAULT_NTP_SERVER);
  settings.advertiseApCredentials = prefs.getBool("apCredsAdv", true);
  settings.labelCharging = prefs.getString("lblChg", "");
  settings.labelStandby = prefs.getString("lblStby", "");
  settings.labelActive = prefs.getString("lblAct", "");
  settings.labelWorking = prefs.getString("lblWork", "");
  settings.hoursBaseline = prefs.getFloat("hrsBase", 0.0f);
  // Hours: migrate from legacy single runtimeSeconds counter into hoursTotal
  const uint64_t legacyRuntime = prefs.getULong64("runtime", 0);
  hoursTotal = prefs.getFloat("hrsTotal", legacyRuntime > 0 ? legacyRuntime / 3600.0f : 0.0f);
  hoursStandby = prefs.getFloat("hrsStby", 0.0f);
  hoursActive = prefs.getFloat("hrsAct", 0.0f);
  hoursWorking = prefs.getFloat("hrsWork", 0.0f);
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
  if (settings.subtitle.isEmpty()) settings.subtitle = DEFAULT_SUBTITLE;
  if (settings.timezone.isEmpty() || settings.timezone == "PST8PDT,M3.2.0,M11.1.0") settings.timezone = DEFAULT_TZ;
  if (settings.ntpServer.isEmpty()) settings.ntpServer = DEFAULT_NTP_SERVER;
  settings.brightness = constrain(settings.brightness, static_cast<uint8_t>(20), static_cast<uint8_t>(255));
  settings.displayRotation = normalizeDisplayRotation(settings.displayRotation);
  settings.lcdTimeoutSec = constrain(settings.lcdTimeoutSec, static_cast<uint16_t>(0), static_cast<uint16_t>(3600));
  settings.idleBleWakeHours = constrain(settings.idleBleWakeHours, 0.25f, 24.0f);
  settings.micRunThreshold = constrain(settings.micRunThreshold, 100.0f, 12000.0f);
  settings.chargeMinAmps = constrain(settings.chargeMinAmps, 0.1f, 10.0f);
  // Populate labels from category defaults if not yet user-defined
  if (settings.labelCharging.isEmpty()) settings.labelCharging = "Recharging";
  if (settings.labelStandby.isEmpty()) settings.labelStandby = activeUsage().standbyMode;
  if (settings.labelActive.isEmpty()) settings.labelActive = activeUsage().activeMode;
  if (settings.labelWorking.isEmpty()) settings.labelWorking = activeUsage().workMode;
  hoursTotal = max(0.0f, hoursTotal);
  settings.hoursBaseline = max(0.0f, settings.hoursBaseline);
  hoursStandby = max(0.0f, hoursStandby);
  hoursActive = max(0.0f, hoursActive);
  hoursWorking = max(0.0f, hoursWorking);
  bool knownProtocol = false;
  for (const auto &profile : BMS_PROFILES) {
    if (settings.bmsProtocol == profile.id) knownProtocol = true;
  }
  if (!knownProtocol) settings.bmsProtocol = BMS_PROFILES[0].id;
}

void saveSettings() {
  prefs.begin("r48disp", false);
  prefs.putString("host", settings.hostname);
  prefs.putString("title", settings.title);
  prefs.putString("timeFmt", settings.timeFormat);
  prefs.putString("apPass", settings.apPassword);
  prefs.putString("otaPass", settings.otaPassword);
  prefs.putString("ssid", settings.wifiSsid);
  prefs.putString("wifiPass", settings.wifiPassword);
  prefs.putString("bmsName", settings.bmsName);
  prefs.putString("bmsProto", settings.bmsProtocol);
  prefs.putString("model", settings.mowerModel);
  prefs.putString("subtitle", settings.subtitle);
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
  prefs.putUShort("lcdTo", settings.lcdTimeoutSec);
  prefs.putBool("pwrSave", settings.powerSaveEnabled);
  prefs.putFloat("idleBleH", settings.idleBleWakeHours);
  prefs.putFloat("lvFloorV", settings.lowVoltageFloorV);
  prefs.putUChar("bbLowPct", settings.boardBatteryLowPct);
  prefs.putBool("mqttEn", settings.mqttEnabled);
  prefs.putString("mqttHost", settings.mqttHost);
  prefs.putUInt("mqttPort", settings.mqttPort);
  prefs.putString("mqttUser", settings.mqttUser);
  prefs.putString("mqttPass", settings.mqttPassword);
  prefs.putString("mqttPfx", settings.mqttTopicPrefix);
  prefs.putBool("mic", settings.featureMic);
  prefs.putFloat("micTh", settings.micRunThreshold);
  prefs.putFloat("chgMinA", settings.chargeMinAmps);
  prefs.putBool("ntpOn", settings.ntpEnabled);
  prefs.putString("ntpHost", settings.ntpServer);
  prefs.putString("lblChg", settings.labelCharging);
  prefs.putString("lblStby", settings.labelStandby);
  prefs.putString("lblAct", settings.labelActive);
  prefs.putString("lblWork", settings.labelWorking);
  prefs.putFloat("hrsBase", settings.hoursBaseline);
  prefs.putBool("apCredsAdv", settings.advertiseApCredentials);
  prefs.end();
}

void cleanupLegacyNvsKeys() {
  prefs.begin("r48disp", false);
  if (prefs.isKey("runtime")) prefs.remove("runtime");
  if (prefs.isKey("vehicle")) prefs.remove("vehicle");
  prefs.end();
}

void saveHours() {
  prefs.begin("r48disp", false);
  prefs.putFloat("hrsTotal", hoursTotal);
  prefs.putFloat("hrsStby", hoursStandby);
  prefs.putFloat("hrsAct", hoursActive);
  prefs.putFloat("hrsWork", hoursWorking);
  prefs.end();
  lastHoursSaveMs = millis();
}

void loadBmsCache() {
  prefs.begin("r48disp", true);
  const bool valid = prefs.getBool("bmsCache", false);
  if (valid) {
    bms.soc = prefs.getUChar("cSoc", 0);
    bms.packVoltage = prefs.getFloat("cVolt", 0.0f);
    bms.currentA = prefs.getFloat("cCurr", 0.0f);
    bms.remainingAh = prefs.getFloat("cRemAh", 0.0f);
    bms.totalAh = prefs.getFloat("cTotAh", 0.0f);
    bms.designAh = prefs.getFloat("cDsgAh", 0.0f);
    bms.deltaCellMv = prefs.getFloat("cDelta", 0.0f);
    bms.cellCount = prefs.getUChar("cCells", 0);
    const size_t bytes = min<size_t>(prefs.getBytesLength("cCellV"), sizeof(bms.cellVolts));
    if (bytes > 0) prefs.getBytes("cCellV", bms.cellVolts, bytes);
    bms.lastAnalogMs = millis();
    cachedAnalogSourceMs = bms.lastAnalogMs;
    bms.status = "cached data, waiting for BLE";
    bmsCacheLoaded = true;
  }
  prefs.end();
}

void saveBmsCache() {
  if (bms.lastAnalogMs == 0 || bms.packVoltage <= 0.1f) return;
  prefs.begin("r48disp", false);
  prefs.putBool("bmsCache", true);
  prefs.putUChar("cSoc", bms.soc);
  prefs.putFloat("cVolt", bms.packVoltage);
  prefs.putFloat("cCurr", bms.currentA);
  prefs.putFloat("cRemAh", bms.remainingAh);
  prefs.putFloat("cTotAh", bms.totalAh);
  prefs.putFloat("cDsgAh", bms.designAh);
  prefs.putFloat("cDelta", bms.deltaCellMv);
  prefs.putUChar("cCells", bms.cellCount);
  prefs.putBytes("cCellV", bms.cellVolts, sizeof(bms.cellVolts));
  prefs.end();
  cachedAnalogSourceMs = bms.lastAnalogMs;
  lastBmsCacheSaveMs = millis();
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

bool initIoExpander() {
  bool ok = tcaWrite(0x03, 0xF0);
  ok = tcaWrite(0x02, 0x00) && ok;
  tcaSetOutput(0x0C);
  delay(30);
  tcaSetOutput(0x0F);
  delay(150);
  if (!ok) Serial.println(F("IO expander: write failed — display may not initialize"));
  return ok;
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
  // Set HIGH immediately so the device survives setup() regardless of settings.
  // applyBatteryHold() corrects this after loadSettings() runs.
  pinMode(PIN_BATTERY_HOLD, OUTPUT);
  digitalWrite(PIN_BATTERY_HOLD, HIGH);
  pinMode(PIN_BATTERY_KEY, INPUT_PULLUP);
}

void applyBatteryHold() {
  digitalWrite(PIN_BATTERY_HOLD, HIGH);
}

void triggerBatteryOff() {
  // Simulate a PMU KEY-button press to toggle battery output off.
  // GPIO 6 (HOLD) going LOW removes the "keep alive" signal, but the PMU
  // only auto-shutoffs when load drops below its minimum — the ESP32 running
  // at full tilt never gets there.  GPIO 7 (KEY) is the actual toggle button:
  // driving it LOW for ~250 ms tells the PMU to cut output immediately.
  digitalWrite(PIN_BATTERY_HOLD, LOW);
  pinMode(PIN_BATTERY_KEY, OUTPUT);
  digitalWrite(PIN_BATTERY_KEY, LOW);
  delay(250);
  // Device will likely lose power here if running on battery.
  // Clean up in case USB is keeping us alive after the battery cut.
  digitalWrite(PIN_BATTERY_KEY, HIGH);
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
  if (screenBattery.present) {
    if (screenBatteryBootVolts < 0.1f) screenBatteryBootVolts = screenBattery.volts;
    if (screenBatteryLowVolts < 0.1f || screenBattery.volts < screenBatteryLowVolts) screenBatteryLowVolts = screenBattery.volts;
  } else {
    screenBatteryLowVolts = 0.0f;
    screenBatteryExternalLatched = true;
    screenBatteryRiseSamples = 0;
  }

  if (screenBattery.present && screenBattery.deltaMv <= BATTERY_EXTERNAL_DROP_MV) {
    screenBatteryExternalLatched = false;
    screenBatteryRiseSamples = 0;
    screenBatteryLowVolts = screenBattery.volts;
  } else if (screenBattery.present && screenBattery.previousVolts > 0.1f && screenBattery.deltaMv >= 5) {
    if (screenBatteryRiseSamples < 4) screenBatteryRiseSamples++;
  } else if (screenBattery.deltaMv < 0) {
    screenBatteryRiseSamples = 0;
  }

  // SOF-only detection: true when a USB host is actively connected (computer, not charger adapters)
  bool usbSofConnected = false;
#if ARDUINO_USB_MODE && ARDUINO_USB_CDC_ON_BOOT
  usbSofConnected = HWCDC::isPlugged();
#endif
  screenBattery.usbSofDetected = usbSofConnected;
  // Broader inference for display/status — battery ≥80% suggests external/charger power
  bool usbPowerPresent = usbSofConnected;
  if (!usbPowerPresent && screenBattery.present && screenBattery.percent >= 80)
    usbPowerPresent = true;
  if (usbPowerPresent || !screenBattery.present ||
      screenBattery.volts >= BATTERY_EXTERNAL_HIGH_V ||
      screenBattery.deltaMv >= BATTERY_EXTERNAL_FAST_RISE_MV ||
      (screenBatteryLowVolts > 0.1f &&
       (screenBattery.volts - screenBatteryLowVolts) >= BATTERY_EXTERNAL_RISE_V &&
       screenBatteryRiseSamples >= 2)) {
    screenBatteryExternalLatched = true;
  }
  const bool externalPowerInferred = screenBatteryExternalLatched && !usbPowerPresent;
  screenBattery.usbCdcConnected = usbPowerPresent;
  if (usbPowerPresent || screenBatteryExternalLatched || !screenBattery.present) {
    powerSource = PowerSource::External;
  } else {
    powerSource = PowerSource::Battery;
  }
  screenBattery.inputStatus = usbPowerPresent ? "USB data/power present"
                              : externalPowerInferred ? "external power inferred"
                              : powerSource == PowerSource::Battery ? "internal battery"
                              : "external power";
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

void syncWifiPowerSave() {
  if (WiFi.getMode() != WIFI_STA || WiFi.status() != WL_CONNECTED) return;
  static bool wifiMaxPs = false;
  const bool wantMax = lastWebRequestMs == 0 || millis() - lastWebRequestMs > 60000UL;
  if (wantMax && !wifiMaxPs) {
    esp_wifi_set_ps(WIFI_PS_MAX_MODEM);
    wifiMaxPs = true;
  } else if (!wantMax && wifiMaxPs) {
    esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
    wifiMaxPs = false;
  }
}

void syncCpuFrequency() {
  const bool wantSlow = displaySleeping &&
    (blePolicyMode == BlePolicyMode::Idle || blePolicyMode == BlePolicyMode::ActiveSlow);
  static bool cpuIsLow = false;
  if (wantSlow && !cpuIsLow) {
    setCpuFrequencyMhz(80);
    cpuIsLow = true;
  } else if (!wantSlow && cpuIsLow) {
    setCpuFrequencyMhz(240);
    cpuIsLow = false;
  }
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
  if (!batteryPowerSaveActive() && displayActivityPresent()) {
    wakeDisplay();
    return;
  }
  const uint32_t timeoutMs = displaySleepTimeoutMs();
  if (timeoutMs > 0 && !displaySleeping && now - lastDisplayActivityMs >= timeoutMs) {
    displaySleeping = true;
    setBacklight(0);
  }
}

void updateHours() {
  const uint32_t now = millis();
  if (lastHoursTickMs == 0) {
    lastHoursTickMs = now;
    lastHoursSaveMs = now;
    return;
  }
  const float deltaH = (now - lastHoursTickMs) / 3600000.0f;
  lastHoursTickMs = now;
  const bool boardBatteryOk = !screenBattery.present || screenBattery.percent >= settings.boardBatteryLowPct;
  if (bms.lastAnalogMs > 0 && boardBatteryOk) {
    hoursTotal += deltaH;
    const ActivityState state = activityState();
    switch (state) {
      case ActivityState::Working:  hoursWorking += deltaH; sessionActiveHours += deltaH; break;
      case ActivityState::Active:   hoursActive += deltaH;  sessionActiveHours += deltaH; break;
      case ActivityState::Charging: /* charging time in total only */                      break;
      default:                      hoursStandby += deltaH;                                break;
    }
  }
  if (now - lastHoursSaveMs >= RUNTIME_SAVE_MS) saveHours();
}

void setHoursTotal(float hours) {
  hoursTotal = max(0.0f, hours - settings.hoursBaseline);
  saveHours();
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
  if (!displayReady || !touchReady) return;
  const bool intLow = digitalRead(PIN_TOUCH_INT) == LOW;

  if (millis() - lastTouchMs < 300) return;
  if (!intLow) return;
  TouchPoint p;
  if (!readTouch(p)) return;
  if (displaySleeping) {
    const uint32_t now = millis();
    const bool doubleTapGesture = p.gesture == 0x0B;
    const bool softwareDoubleTap = lastSleepTapMs > 0 && now - lastSleepTapMs <= TOUCH_DOUBLE_TAP_MS;
    lastTouchMs = now;
    if (doubleTapGesture || softwareDoubleTap) {
      lastSleepTapMs = 0;
      wakeDisplay();
    } else {
      lastSleepTapMs = now;
    }
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
  if (settings.ntpEnabled && !settings.ntpServer.isEmpty()) {
    configTzTime(settings.timezone.c_str(), settings.ntpServer.c_str());
    ntpConfigured = true;
  } else {
    setenv("TZ", settings.timezone.c_str(), 1);
    tzset();
    ntpConfigured = false;
  }
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
  if (WiFi.status() == WL_CONNECTED && settings.ntpEnabled && !ntpConfigured) configureClock();
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
  s.totalAh = effectiveTotalAh();
  s.designAh = effectiveDesignAh();
  s.cellCount = bms.cellCount;
  s.mode = mowerModeLabel();
  s.runEta = formatHours(runtimeEstimateHours());
  s.chargeEta = packCharging() ? formatHours(chargeEstimateHours()) : "--";
  s.rideTime = formatHours(sessionActiveHours);
  s.temp = formatTemp(bms.pcbTempC);
  s.runtimeHours = String(settings.hoursBaseline + hoursTotal, 1) + " h";
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
  s.bmsStatus = bms.status;
  s.screenBattery = screenBattery.present ? String(screenBattery.volts, 2) + "V " + String(screenBattery.percent) + "%" : "no battery";
  s.powerSource = powerSourceName(powerSource);
  s.mqttStatus = settings.mqttEnabled ? String("MQTT ") + mqttClient.statusLabel() : String("");
  s.maintOverdue = maintOverdueCount();
  s.touchReady = touchReady;
  s.hoursActStr = String(hoursActive, 1) + "h act";
  s.hoursWorkStr = String(hoursWorking, 1) + "h wrk";
  s.use24h = settings.timeFormat == "24h";
  s.powerSaveEnabled = settings.powerSaveEnabled;
  s.apPassword = settings.apPassword;
  s.advertiseApCreds = settings.advertiseApCredentials;
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
  const String pageTitle = displayTitle();
  html += escapeHtml(pageTitle);
  html += F("</title></head><body><header><div class='brand'><span class='mark'></span><div><h1>");
  html += escapeHtml(pageTitle);
  html += F("</h1><p>");
  html += escapeHtml(settings.subtitle);
  html += F(" - ");
  html += escapeHtml(title);
  html += F("</p></div></div><nav><a href='/'>Dashboard</a><a href='/battery'>Battery</a><a href='/maintenance'>Maintenance</a><a href='/settings'>Settings</a></nav></header><main class='wrap'>");
  return html;
}

String commonFoot() {
  String html = F("</main><script src='/app.js'></script></body></html>");
  return html;
}

String themeCss() {
  const ThemeProfile &theme = activeTheme();
  String css;
  css.reserve(7200);
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
           "nav{display:flex;gap:8px;flex-wrap:wrap}a,button,input,select{font:inherit}nav a,button{border:1px solid var(--line);background:var(--panel2);color:var(--text);border-radius:7px;padding:8px 11px;text-decoration:none;cursor:pointer}button.primary{background:var(--primary);border-color:var(--primary);color:var(--buttonText);font-weight:800}button.btn-danger{background:var(--bad);border-color:var(--bad);color:#fff;font-weight:700}"
           ".wrap{max-width:1180px;margin:0 auto;padding:18px;display:grid;gap:16px}.toolbar{display:flex;gap:8px;flex-wrap:wrap}.grid,.dashboard-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(210px,1fr));gap:12px}.wide{grid-column:1/-1}.card,.metric,.gauge-card{background:var(--panel);border:1px solid var(--line);border-radius:8px;padding:14px}.metric{min-height:104px}.label{color:var(--muted);font-size:13px}.value{font-size:30px;margin-top:6px}.value.sm{font-size:20px;line-height:1.3}.subline{color:var(--muted);text-align:center;margin-top:12px}"
           ".gauge-card{display:grid;place-items:center}.gauge{width:min(60vw,230px);aspect-ratio:1;border-radius:50%;display:grid;place-items:center;position:relative;background:conic-gradient(from 135deg,var(--primary) calc(var(--pct)*0.75%),var(--line) 0 75%,transparent 75%)}.gauge:before{content:'';position:absolute;inset:22px;border-radius:50%;background:var(--panel)}.gauge b{position:relative;font-size:42px;font-weight:700;color:var(--primary)}"
           ".section-head{display:flex;align-items:center;justify-content:space-between;gap:10px;margin-bottom:12px}.form-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(220px,1fr));gap:12px}label{display:grid;gap:6px;color:var(--muted);font-size:13px}label.check{display:flex;align-items:center;gap:8px;color:var(--text)}label.check input{width:auto}.hint{color:var(--muted);font-size:12px;line-height:1.35}input,select{width:100%;background:var(--panel2);color:var(--text);border:1px solid var(--line);border-radius:7px;padding:9px}.actions{display:flex;gap:8px;flex-wrap:wrap;margin-bottom:12px}"
           ".cells{display:grid;grid-template-columns:repeat(auto-fit,minmax(132px,1fr));gap:8px}.cell{background:var(--panel2);border:1px solid var(--line);border-radius:7px;padding:9px;display:grid;gap:6px}.cell span{color:var(--muted);font-size:12px}.cell b{font-size:18px}.cell i{display:block;height:6px;border-radius:6px;background:linear-gradient(90deg,var(--primary2),var(--primary));width:var(--w)}.list{display:grid;gap:8px}.list button{display:flex;justify-content:space-between;text-align:left}.list span{color:var(--muted)}.empty,.note{color:var(--muted);line-height:1.45}table{width:100%;border-collapse:collapse;background:var(--panel);border:1px solid var(--line)}td,th{padding:8px;border-bottom:1px solid var(--line);text-align:left;vertical-align:top}th{color:var(--muted);font-weight:500}.bms-pack{display:grid;grid-template-columns:minmax(190px,270px) 1fr;gap:12px;align-items:center;background:var(--panel);border:1px solid var(--line);border-radius:8px;padding:14px}.pack-cols{display:grid;grid-template-columns:repeat(auto-fit,minmax(150px,1fr));gap:10px}@media(max-width:760px){header{align-items:flex-start;flex-direction:column}.bms-pack{grid-template-columns:1fr}}");
  css += F(
      ".dashboard-panel{display:grid;grid-template-columns:minmax(190px,270px) 1fr;gap:14px;align-items:center;background:var(--panel);border:1px solid var(--line);border-radius:8px;padding:14px;overflow:hidden}"
      ".dashboard-panel .gauge-card{background:transparent;border:0;padding:0;min-width:0}.dashboard-gauge{align-self:stretch}.dashboard-panel .gauge{width:min(100%,230px);max-width:230px}"
      ".dash-stats,.dash-strip{display:grid;grid-template-columns:repeat(auto-fit,minmax(150px,1fr));gap:10px}.dash-strip{align-items:stretch}.dash-stats .metric,.dash-strip .metric{min-width:0}"
      ".metric,.gauge-card{overflow:hidden;min-width:0}.value{overflow-wrap:anywhere}.bms-pack .gauge-card{min-width:0}.bms-pack .gauge{width:min(100%,230px);max-width:230px}"
      ".threshold-grid{display:grid;grid-template-columns:minmax(170px,.8fr) minmax(220px,1.2fr);gap:12px;align-items:end}.threshold-grid input:disabled{opacity:.72}"
      ".settings-group{border-top:1px solid var(--line);padding-top:16px;margin-top:16px}.settings-group h3{margin:0 0 12px;font-size:15px;color:var(--primary)}"
      ".ble-device{background:var(--panel2);border:1px solid var(--line);border-radius:7px;padding:8px}.ble-name{font-size:14px;margin-bottom:6px;display:flex;justify-content:space-between;align-items:center}.ble-name span{color:var(--muted);font-size:12px}.ble-protos{display:flex;gap:6px;flex-wrap:wrap}.ble-proto{padding:4px 9px;font-size:12px}"
      "@media(max-width:760px){.dashboard-panel{grid-template-columns:1fr}.threshold-grid{grid-template-columns:1fr}.dash-stats,.dash-strip{grid-template-columns:1fr}}");
  return css;
}

void sendContentPieces(const String &content, size_t chunkSize = 1024) {
  WiFiClient client = server.client();
  const uint8_t *data = reinterpret_cast<const uint8_t *>(content.c_str());
  const size_t total = content.length();
  for (size_t offset = 0; offset < total; offset += chunkSize) {
    const size_t len = min(chunkSize, total - offset);
    client.write(data + offset, len);
    delay(0);
  }
}

void sendPage(const String &title, String (*body)()) {
  lastWebRequestMs = millis();
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
  out.reserve(measureJson(doc) + 4);
  serializeJson(doc, out);
  server.sendHeader("Connection", "close");
  server.send(200, "application/json", out);
}

void addStatusJson(JsonDocument &doc) {
  doc["firmware"] = FIRMWARE_VERSION;
  doc["project"] = PROJECT_NAME;
  doc["uptime_ms"] = millis();
  doc["uptime"] = formatDuration(millis() / 1000ULL);
  doc["hostname"] = settings.hostname;
  doc["title"] = displayTitle();
  doc["subtitle"] = settings.subtitle;

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

  const ActivityState aState = activityState();
  JsonObject vehicle = doc["vehicle"].to<JsonObject>();
  vehicle["label"] = settings.mowerModel;
  vehicle["usage_category"] = settings.usageCategory;
  vehicle["usage_label"] = usage.label;
  vehicle["activity_state"] = activityStateKey(aState);
  vehicle["activity_label"] = stateLabel(aState);
  vehicle["mode"] = mowerModeLabel();
  vehicle["active"] = activityDetected();
  vehicle["work_likely"] = workDetected();
  vehicle["audio_work_tone"] = usage.audioAssist && R48Mic::toneDetected();
  vehicle["activity_detection"] = settings.activityDetection;
  vehicle["work_detection"] = settings.workDetection;
  vehicle["charge_min_amps"] = serialized(String(settings.chargeMinAmps, 1));
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

  JsonObject hours = doc["hours"].to<JsonObject>();
  hours["baseline"] = serialized(String(settings.hoursBaseline, 2));
  hours["counted"] = serialized(String(hoursTotal, 2));
  hours["total"] = serialized(String(settings.hoursBaseline + hoursTotal, 2));
  hours["standby"] = serialized(String(hoursStandby, 2));
  hours["active"] = serialized(String(hoursActive, 2));
  hours["working"] = serialized(String(hoursWorking, 2));
  hours["session_active"] = serialized(String(sessionActiveHours, 2));

  // Keep mower object for backward compatibility with any existing consumers
  JsonObject mower = doc["mower"].to<JsonObject>();
  mower["model"] = settings.mowerModel;
  mower["mode"] = mowerModeLabel();
  mower["activity_state"] = activityStateKey(aState);
  mower["runtime_hours"] = serialized(String(settings.hoursBaseline + hoursTotal, 2));
  mower["mower_running"] = activityDetected();
  mower["blades_likely_on"] = workDetected();
  mower["mic_mower_tone"] = usage.audioAssist && R48Mic::toneDetected();
  mower["run_threshold_a"] = serialized(String(settings.mowerRunAmps, 1));
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
  b["policy"] = blePolicyName(blePolicyMode);
  b["soc_rate_pct_per_hour"] = serialized(String(socRatePctPerHour, 2));
  b["cache_loaded"] = bmsCacheLoaded;
  b["last_error"] = bms.lastError;
  b["connected"] = bms.connected;
  b["authenticated"] = bms.authenticated;
  b["scanning"] = bms.scanning;
  b["rssi"] = bms.rssi;
  b["frames"] = bms.frames;
  b["bytes"] = bms.bytes;
  b["errors"] = bms.errors;
  b["last_analog_age"] = millisAge(bms.lastAnalogMs);
  b["last_rx_age"] = millisAge(bms.lastRxMs);
  b["last_connect_age"] = millisAge(bms.lastConnectMs);
  b["frame_hex"] = bms.latestFrameHex;
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
  b["found"] = bms.found;
  b["last_warning_age"] = millisAge(bms.lastWarningMs);
  b["warning_hex"] = bms.warningHex;
  b["balance_state"] = bms.balanceState;
  b["balance_current_a"] = serialized(String(bms.balanceCurrentA, 2));
  b["health_percent"] = serialized(String(bmsHealthPercent(), 1));
  JsonArray cells = b["cells"].to<JsonArray>();
  for (uint8_t i = 0; i < bms.cellCount && i < 32; ++i) cells.add(serialized(String(bms.cellVolts[i], 3)));
  JsonArray temps = b["temps"].to<JsonArray>();
  auto addTemp = [&](float c) {
    if (fabsf(c) < 0.5f) temps.add(String("--"));
    else temps.add(serialized(String(displayTempC(c), 1)));
  };
  if (bms.mosTempC != 0.0f || bms.tempCount > 0) addTemp(bms.mosTempC);
  if (bms.pcbTempC != 0.0f || bms.tempCount > 1) addTemp(bms.pcbTempC);
  for (uint8_t i = 0; i < bms.tempCount && i < 14; ++i) addTemp(bms.tempsC[i]);

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
  nvs_stats_t nvsStats;
  if (nvs_get_stats(nullptr, &nvsStats) == ESP_OK) {
    JsonObject nvs = hardware["nvs"].to<JsonObject>();
    nvs["used_entries"] = nvsStats.used_entries;
    nvs["free_entries"] = nvsStats.free_entries;
    nvs["total_entries"] = nvsStats.total_entries;
    nvs["pct_used"] = nvsStats.total_entries > 0
        ? serialized(String(100.0f * nvsStats.used_entries / nvsStats.total_entries, 1))
        : serialized(String("0.0"));
  }

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
  display["sleep_timeout_ms"] = displaySleepTimeoutMs();
  display["sleep_timeout_sec"] = settings.lcdTimeoutSec;

  JsonObject espBat = doc["screen_battery"].to<JsonObject>();
  espBat["present"] = screenBattery.present;
  espBat["voltage"] = serialized(String(screenBattery.volts, 3));
  espBat["delta_mv"] = screenBattery.deltaMv;
  espBat["boot_voltage"] = serialized(String(screenBatteryBootVolts, 3));
  espBat["low_voltage"] = serialized(String(screenBatteryLowVolts, 3));
  espBat["rise_samples"] = screenBatteryRiseSamples;
  espBat["external_latched"] = screenBatteryExternalLatched;
  espBat["percent"] = screenBattery.percent;
  espBat["label"] = screenBattery.label;
  espBat["input_status"] = screenBattery.inputStatus;
  espBat["usb_detected"] = screenBattery.usbCdcConnected;
  espBat["power_source"] = powerSourceName(powerSource);
  espBat["power_save_enabled"] = settings.powerSaveEnabled;
  espBat["battery_power_saving"] = batteryPowerSaveActive();

  JsonObject mqttObj = doc["mqtt"].to<JsonObject>();
  mqttObj["enabled"] = settings.mqttEnabled;
  mqttObj["connected"] = mqttClient.connected();
  mqttObj["status"] = mqttClient.statusLabel();
  mqttObj["broker"] = settings.mqttEnabled && !settings.mqttHost.isEmpty()
    ? settings.mqttHost + ":" + String(settings.mqttPort) : String("");

  JsonObject degr = doc["degradation"].to<JsonObject>();
  degr["cycle_count"] = bms.cycleCount;
  const float designAh = max(0.01f, bms.found ? bms.designAh * capacityScale() : settings.nominalPackAh);
  const float totalAh = bms.found ? bms.totalAh * capacityScale() : 0.0f;
  const float fadePct = max(0.0f, (designAh - totalAh) / designAh * 100.0f);
  degr["capacity_fade_pct"] = serialized(String(fadePct, 1));
  degr["max_cell_spread_mv"] = serialized(String(degradation.maxCellSpreadMv, 1));
  degr["min_cell_voltage_v"] = degradation.minCellVoltageV < 9.0f ? serialized(String(degradation.minCellVoltageV, 3)) : serialized(String(0.0f, 3));
  degr["max_temp_c"] = degradation.maxTempC > -98.0f ? serialized(String(degradation.maxTempC, 1)) : serialized(String(0.0f, 1));
  degr["low_voltage_events"] = degradation.lowVoltageEvents;
  degr["high_current_events"] = degradation.highCurrentEvents;
  degr["low_voltage_floor_v"] = serialized(String(settings.lowVoltageFloorV, 2));

  JsonObject clock = doc["clock"].to<JsonObject>();
  clock["ntp_enabled"] = settings.ntpEnabled;
  clock["ntp_configured"] = ntpConfigured;
  clock["ntp_server"] = settings.ntpEnabled ? settings.ntpServer : String("");
  clock["timezone"] = settings.timezone;
  clock["local_time"] = currentTimeText("%Y-%m-%d %H:%M:%S");
}

void apiStatus() {
  lastWebRequestMs = millis();
  JsonDocument doc;
  addStatusJson(doc);
  sendJson(doc);
}

void apiSettingsGet() {
  JsonDocument doc;
  doc["hostname"] = settings.hostname;
  doc["title"] = settings.title;
  doc["time_format"] = settings.timeFormat;
  doc["ap_ssid"] = apSsid;
  doc["ap_password_set"] = settings.apPassword.length() >= 8;
  doc["advertise_ap_credentials"] = settings.advertiseApCredentials;
  doc["ota_password_set"] = settings.otaPassword.length() >= 8;
  doc["wifi_ssid"] = settings.wifiSsid;
  doc["wifi_password_set"] = !settings.wifiPassword.isEmpty();
  doc["bms_name"] = settings.bmsName;
  doc["bms_protocol"] = settings.bmsProtocol;
  doc["mower_model"] = settings.mowerModel;
  doc["subtitle"] = settings.subtitle;
  doc["usage_category"] = settings.usageCategory;
  doc["vehicle_type"] = settings.usageCategory;
  doc["theme_id"] = settings.themeId;
  doc["discharge_current_negative"] = settings.dischargeCurrentNegative;
  doc["display_enabled"] = settings.displayEnabled;
  doc["activity_detection"] = settings.activityDetection;
  doc["work_detection"] = settings.workDetection;
  doc["typical_mow_amps"] = settings.typicalMowAmps;
  doc["mower_run_amps"] = settings.mowerRunAmps;
  doc["activity_amps"] = settings.mowerRunAmps;
  doc["mowing_detect_amps"] = settings.bladesOnAmps;
  doc["work_amps"] = settings.bladesOnAmps;
  doc["charge_min_amps"] = settings.chargeMinAmps;
  doc["label_charging"] = settings.labelCharging;
  doc["label_standby"] = settings.labelStandby;
  doc["label_active"] = settings.labelActive;
  doc["label_working"] = settings.labelWorking;
  doc["nominal_pack_ah"] = settings.nominalPackAh;
  doc["temp_unit"] = settings.tempUnit;
  doc["timezone"] = settings.timezone;
  doc["ntp_enabled"] = settings.ntpEnabled;
  doc["ntp_server"] = settings.ntpServer;
  doc["brightness"] = settings.brightness;
  doc["display_rotation"] = settings.displayRotation;
  doc["lcd_timeout_sec"] = settings.lcdTimeoutSec;
  doc["power_save_enabled"] = settings.powerSaveEnabled;
  doc["idle_ble_wake_hours"] = serialized(String(settings.idleBleWakeHours, 2));
  doc["low_voltage_floor_v"] = serialized(String(settings.lowVoltageFloorV, 2));
  doc["board_battery_low_pct"] = settings.boardBatteryLowPct;
  doc["mqtt_enabled"] = settings.mqttEnabled;
  doc["mqtt_host"] = settings.mqttHost;
  doc["mqtt_port"] = settings.mqttPort;
  doc["mqtt_user"] = settings.mqttUser;
  doc["mqtt_password_set"] = !settings.mqttPassword.isEmpty();
  doc["mqtt_topic_prefix"] = settings.mqttTopicPrefix;
  doc["feature_mic"] = settings.featureMic;
  doc["mic_run_threshold"] = serialized(String(settings.micRunThreshold, 0));
  doc["hours_baseline"] = serialized(String(settings.hoursBaseline, 2));
  doc["hours_total"] = serialized(String(settings.hoursBaseline + hoursTotal, 2));
  doc["hours_counted"] = serialized(String(hoursTotal, 2));
  doc["hours_active"] = serialized(String(hoursActive, 2));
  doc["hours_working"] = serialized(String(hoursWorking, 2));
  sendJson(doc);
}

void apiSettingsPost() {
  const String oldBms = settings.bmsName;
  const String oldProto = settings.bmsProtocol;
  const String oldSsid = settings.wifiSsid;
  const uint16_t oldRotation = settings.displayRotation;
  const String oldUsage = settings.usageCategory;
  const String oldTimezone = settings.timezone;
  const bool oldNtpEnabled = settings.ntpEnabled;
  const String oldNtpServer = settings.ntpServer;
  if (server.hasArg("hostname")) settings.hostname = sanitizeHostname(server.arg("hostname"));
  if (server.hasArg("title")) { settings.title = server.arg("title"); settings.title.trim(); if (settings.title.length() > 64) settings.title = settings.title.substring(0, 64); }
  if (server.hasArg("ota_password") && server.arg("ota_password").length() >= 8) settings.otaPassword = server.arg("ota_password");
  if (server.hasArg("ap_password") && server.arg("ap_password").length() >= 8) settings.apPassword = server.arg("ap_password");
  if (server.hasArg("advertise_ap_credentials")) settings.advertiseApCredentials = server.arg("advertise_ap_credentials") == "1";
  if (server.hasArg("wifi_ssid")) settings.wifiSsid = server.arg("wifi_ssid");
  if (server.hasArg("wifi_password") && server.arg("wifi_password").length() > 0) settings.wifiPassword = server.arg("wifi_password");
  if (server.hasArg("bms_name")) settings.bmsName = server.arg("bms_name");
  if (server.hasArg("bms_protocol")) settings.bmsProtocol = server.arg("bms_protocol");
  if (server.hasArg("mower_model")) settings.mowerModel = server.arg("mower_model");
  if (server.hasArg("subtitle")) settings.subtitle = server.arg("subtitle");
  if (server.hasArg("usage_category")) settings.usageCategory = server.arg("usage_category");
  else if (server.hasArg("vehicle_type")) settings.usageCategory = server.arg("vehicle_type");
  if (server.hasArg("theme_id")) settings.themeId = server.arg("theme_id");
  if (server.hasArg("discharge_current_negative")) settings.dischargeCurrentNegative = server.arg("discharge_current_negative") == "1";
  if (server.hasArg("display_enabled")) settings.displayEnabled = server.arg("display_enabled") == "1";
  if (server.hasArg("activity_detection")) settings.activityDetection = server.arg("activity_detection") == "1";
  if (server.hasArg("work_detection")) settings.workDetection = server.arg("work_detection") == "1";
  if (server.hasArg("typical_mow_amps")) settings.typicalMowAmps = constrain(server.arg("typical_mow_amps").toFloat(), 5.0f, 300.0f);
  if (server.hasArg("mower_run_amps")) settings.mowerRunAmps = constrain(server.arg("mower_run_amps").toFloat(), 1.0f, 300.0f);
  if (server.hasArg("mowing_detect_amps")) settings.bladesOnAmps = constrain(server.arg("mowing_detect_amps").toFloat(), settings.mowerRunAmps, 400.0f);
  if (server.hasArg("nominal_pack_ah")) settings.nominalPackAh = constrain(server.arg("nominal_pack_ah").toFloat(), 1.0f, 400.0f);
  if (server.hasArg("timezone")) settings.timezone = server.arg("timezone");
  if (server.hasArg("brightness")) settings.brightness = constrain(static_cast<uint8_t>(server.arg("brightness").toInt()), static_cast<uint8_t>(20), static_cast<uint8_t>(255));
  if (server.hasArg("display_rotation")) settings.displayRotation = normalizeDisplayRotation(server.arg("display_rotation").toInt());
  if (server.hasArg("lcd_timeout_sec")) settings.lcdTimeoutSec = constrain(static_cast<uint16_t>(server.arg("lcd_timeout_sec").toInt()), static_cast<uint16_t>(0), static_cast<uint16_t>(3600));
  if (server.hasArg("power_save_enabled")) {
    settings.powerSaveEnabled = server.arg("power_save_enabled") == "1" || server.arg("power_save_enabled") == "true" || server.arg("power_save_enabled") == "on";
    applyBatteryHold();
  }
  if (server.hasArg("idle_ble_wake_hours")) settings.idleBleWakeHours = constrain(server.arg("idle_ble_wake_hours").toFloat(), 0.25f, 24.0f);
  if (server.hasArg("low_voltage_floor_v")) settings.lowVoltageFloorV = constrain(server.arg("low_voltage_floor_v").toFloat(), 2.0f, 3.8f);
  if (server.hasArg("board_battery_low_pct")) settings.boardBatteryLowPct = (uint8_t)constrain(server.arg("board_battery_low_pct").toInt(), 5, 80);
  if (server.hasArg("mqtt_enabled")) settings.mqttEnabled = server.arg("mqtt_enabled") == "1" || server.arg("mqtt_enabled") == "on" || server.arg("mqtt_enabled") == "true";
  if (server.hasArg("mqtt_host")) settings.mqttHost = server.arg("mqtt_host");
  if (server.hasArg("mqtt_port")) settings.mqttPort = static_cast<uint16_t>(constrain(server.arg("mqtt_port").toInt(), 1, 65535));
  if (server.hasArg("mqtt_user")) settings.mqttUser = server.arg("mqtt_user");
  if (server.hasArg("mqtt_password") && server.arg("mqtt_password").length() > 0) settings.mqttPassword = server.arg("mqtt_password");
  if (server.hasArg("mqtt_topic_prefix")) settings.mqttTopicPrefix = server.arg("mqtt_topic_prefix");
  if (server.hasArg("feature_mic")) settings.featureMic = server.arg("feature_mic") == "1";
  if (server.hasArg("mic_run_threshold")) settings.micRunThreshold = constrain(server.arg("mic_run_threshold").toFloat(), 100.0f, 12000.0f);
  if (server.hasArg("charge_min_amps")) settings.chargeMinAmps = constrain(server.arg("charge_min_amps").toFloat(), 0.1f, 200.0f);
  if (server.hasArg("label_charging") && server.arg("label_charging").length() > 0) settings.labelCharging = server.arg("label_charging");
  if (server.hasArg("label_standby") && server.arg("label_standby").length() > 0) settings.labelStandby = server.arg("label_standby");
  if (server.hasArg("label_active") && server.arg("label_active").length() > 0) settings.labelActive = server.arg("label_active");
  if (server.hasArg("label_working") && server.arg("label_working").length() > 0) settings.labelWorking = server.arg("label_working");
  if (server.hasArg("ntp_enabled")) settings.ntpEnabled = server.arg("ntp_enabled") == "1";
  if (server.hasArg("ntp_server")) settings.ntpServer = server.arg("ntp_server");
  if (server.hasArg("time_format")) settings.timeFormat = server.arg("time_format") == "24h" ? "24h" : "12h";
  if (server.hasArg("temp_unit")) { const String u = server.arg("temp_unit"); settings.tempUnit = (u == "C") ? "C" : "F"; }
  // Changing baseline adjusts the displayed total without touching counted hours.
  // Skipping setHoursTotal when baseline changes avoids hours_total (= old_baseline + counted)
  // being misinterpreted as the new total and zeroing the counted portion.
  const bool baselineChanged = server.hasArg("hours_baseline") && server.arg("hours_baseline").length() > 0;
  if (baselineChanged) settings.hoursBaseline = max(0.0f, server.arg("hours_baseline").toFloat());
  if (!baselineChanged) {
    if (server.hasArg("hours_total")) setHoursTotal(server.arg("hours_total").toFloat());
    if (server.hasArg("runtime_hours")) setHoursTotal(server.arg("runtime_hours").toFloat());
  }
  if (server.hasArg("hours_active") && server.arg("hours_active").length() > 0)
    hoursActive = max(0.0f, server.arg("hours_active").toFloat());
  if (server.hasArg("hours_working") && server.arg("hours_working").length() > 0)
    hoursWorking = max(0.0f, server.arg("hours_working").toFloat());
  if (baselineChanged || server.hasArg("hours_active") || server.hasArg("hours_working")) saveHours();
  if (!validUsageCategory(settings.usageCategory)) settings.usageCategory = USAGE_CATEGORIES[0].id;
  if (oldUsage != settings.usageCategory) {
    if (!server.hasArg("theme_id")) settings.themeId = activeUsage().defaultThemeId;
    // Reset labels to new category defaults unless the user also sent new labels this request
    if (!server.hasArg("label_standby")) settings.labelStandby = activeUsage().standbyMode;
    if (!server.hasArg("label_active")) settings.labelActive = activeUsage().activeMode;
    if (!server.hasArg("label_working")) settings.labelWorking = activeUsage().workMode;
  }
  if (!validThemeId(settings.themeId)) settings.themeId = activeUsage().defaultThemeId;
  if (settings.subtitle.isEmpty()) settings.subtitle = DEFAULT_SUBTITLE;
  if (settings.timezone.isEmpty() || settings.timezone == "PST8PDT,M3.2.0,M11.1.0") settings.timezone = DEFAULT_TZ;
  if (settings.ntpServer.isEmpty()) settings.ntpServer = DEFAULT_NTP_SERVER;
  if (settings.bladesOnAmps < settings.mowerRunAmps) settings.bladesOnAmps = settings.mowerRunAmps;
  if (settings.apPassword.length() < 8) settings.apPassword = "r48display";
  if (settings.otaPassword.length() < 8) settings.otaPassword = "r48display";
  saveSettings();
  R48Mic::configure(settings.featureMic, settings.micRunThreshold);
  if (oldRotation != settings.displayRotation && displayReady) {
    applyDisplayRotation();
    gfx->fillScreen(COLOR_BG);
  }
  setBacklight(settings.displayEnabled ? settings.brightness : 0);
  if (oldTimezone != settings.timezone || oldNtpEnabled != settings.ntpEnabled || oldNtpServer != settings.ntpServer) {
    ntpConfigured = false;
    configureClock();
  }
  if (oldBms != settings.bmsName || oldProto != settings.bmsProtocol) bleBms.reconnect();
  if (oldSsid != settings.wifiSsid && !settings.wifiSsid.isEmpty()) pendingStaStart = true;
  setupMqtt();
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
  if (count < 0) {
    WiFi.scanDelete();
    if (oldMode == WIFI_AP) WiFi.mode(WIFI_AP);
    server.send(503, "application/json", "{\"networks\":[],\"error\":\"wifi scan failed\"}");
    return;
  }
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

void apiBatteryOff() {
  if (!screenBattery.present) {
    server.send(400, "application/json", "{\"ok\":false,\"error\":\"No internal battery detected\"}");
    return;
  }
  server.sendHeader("Connection", "close");
  server.send(200, "application/json", "{\"ok\":true,\"note\":\"Cutting battery power\"}");
  delay(300);
  saveSettings(); saveHours(); saveMaintenance(); saveDegradation();
  triggerBatteryOff();
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
  server.on("/maintenance", HTTP_GET, []() { sendPage("Maintenance", R48Web::maintenanceBody); });
  server.on("/settings", HTTP_GET, []() { sendPage("Settings", R48Web::settingsBody); });
  server.on("/status", HTTP_GET, []() { server.sendHeader("Location", "/", true); server.send(302, "text/plain", ""); });
  server.on("/update", HTTP_GET, []() { server.sendHeader("Location", "/settings"); server.send(302); });
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
  server.on("/api/battery/off", HTTP_POST, apiBatteryOff);
  server.on("/api/maintenance", HTTP_GET, apiMaintenanceGet);
  server.on("/api/maintenance", HTTP_POST, apiMaintenancePost);
  server.on("/api/maintenance/confirm", HTTP_POST, apiMaintenanceConfirm);
  server.on("/api/maintenance/delete", HTTP_POST, apiMaintenanceDelete);
  server.on("/api/maintenance/history", HTTP_GET, apiMaintenanceHistoryGet);
  server.on("/api/maintenance/history/delete", HTTP_POST, apiMaintenanceHistoryDelete);
  server.on("/api/maintenance/export", HTTP_GET, apiMaintenanceExport);
  server.on("/api/mqtt/publish-now", HTTP_POST, apiMqttPublishNow);
  server.on("/api/mqtt/rediscover", HTTP_POST, apiMqttRediscover);
  server.on("/api/mqtt/test", HTTP_GET, apiMqttTest);
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
  {
    const esp_reset_reason_t reason = esp_reset_reason();
    const char *reasonName = "unknown";
    switch (reason) {
      case ESP_RST_POWERON:  reasonName = "power-on";          break;
      case ESP_RST_EXT:      reasonName = "external";          break;
      case ESP_RST_SW:       reasonName = "software";          break;
      case ESP_RST_PANIC:    reasonName = "panic";             break;
      case ESP_RST_INT_WDT:  reasonName = "interrupt watchdog";break;
      case ESP_RST_TASK_WDT: reasonName = "task watchdog";     break;
      case ESP_RST_WDT:      reasonName = "other watchdog";    break;
      case ESP_RST_DEEPSLEEP:reasonName = "deep sleep";        break;
      case ESP_RST_BROWNOUT: reasonName = "brownout";          break;
      default: break;
    }
    Serial.printf("Reset reason: %s\n", reasonName);
  }
  if (psramFound()) {
    heap_caps_malloc_extmem_enable(4096);
    Serial.printf("PSRAM enabled: %u bytes\n", static_cast<unsigned>(ESP.getPsramSize()));
  } else {
    Serial.println(F("PSRAM not detected"));
  }
  loadSettings();
  // Write any keys that didn't exist yet (new fields added by firmware update).
  // This ensures title, hoursBaseline, and other recently-added keys are present
  // in NVS even if the device hasn't been through the settings form since the upgrade.
  saveSettings();
  cleanupLegacyNvsKeys();
  {
    nvs_stats_t nvsStats;
    if (nvs_get_stats(nullptr, &nvsStats) == ESP_OK) {
      Serial.printf("NVS: %u/%u entries used (%.1f%%), %u free\n",
                    static_cast<unsigned>(nvsStats.used_entries),
                    static_cast<unsigned>(nvsStats.total_entries),
                    nvsStats.total_entries > 0 ? 100.0f * nvsStats.used_entries / nvsStats.total_entries : 0.0f,
                    static_cast<unsigned>(nvsStats.free_entries));
    }
  }
  applyBatteryHold();
  setupMqtt();
  loadDegradation();
  loadMaintenance();
  initDefaultMaintenanceItems();
  loadBmsCache();
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  Wire.setClock(400000);
  initDisplay();
  analogReadResolution(12);
  analogSetPinAttenuation(PIN_BATTERY_ADC, ADC_11db);
  updateScreenBattery();
  R48Mic::begin(settings.featureMic, settings.micRunThreshold);
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
  updateSocRate();
  updateDegradation();
  updateMqtt();
  if (bms.lastAnalogMs > 0 && bms.lastAnalogMs != cachedAnalogSourceMs &&
      (lastBmsCacheSaveMs == 0 || millis() - lastBmsCacheSaveMs > 300000UL)) {
    saveBmsCache();
  }
  R48Mic::loop(activityDetected());
  updateHours();
  maybeProbeTouch();
  maybeHandleTouch();
  pollButtons();
  R48DisplayUi::tick(displaySleeping);
  autoDisplayMode();
  manageDisplayPower();
  syncCpuFrequency();
  syncWifiPowerSave();

  const uint32_t now = millis();
  if (now - lastBatteryMs >= BATTERY_REFRESH_MS) {
    lastBatteryMs = now;
    updateScreenBattery();
    // Shutdown when external power is lost and internal battery backup is disabled.
    // Two detection paths prevent boot-loop false triggers (firing before USB settles):
    //   SOF path: USB host (computer) sends SOF → removal detected instantly
    //   Inference path: battery ≥80% implies charger/USB power → fires when battery
    //                   drops below 80% after being above it (slower, for DCP adapters)
    // Each path tracks "was ever seen" so the timer only starts on a real transition.
    static bool usbSofEverSeen = false;
    static bool usbInferenceEverSeen = false;
    static uint32_t shutdownGoneMs = 0;
    if (screenBattery.usbSofDetected) usbSofEverSeen = true;
    if (screenBattery.usbCdcConnected) usbInferenceEverSeen = true;
    const bool sofGone = usbSofEverSeen && !screenBattery.usbSofDetected;
    const bool inferenceGone = usbInferenceEverSeen && !screenBattery.usbCdcConnected;
    if (screenBattery.present && (sofGone || inferenceGone)) {
      if (shutdownGoneMs == 0) shutdownGoneMs = now;
      else if (!settings.powerSaveEnabled && now - shutdownGoneMs > 8000UL) {
        saveSettings(); saveHours(); saveMaintenance(); saveDegradation();
        triggerBatteryOff();
      }
    } else {
      shutdownGoneMs = 0;
    }
    // Graceful shutdown on critically low board battery
    if (screenBattery.present && !screenBattery.usbCdcConnected &&
        screenBattery.percent <= 4 && screenBattery.volts < 3.4f) {
      saveSettings(); saveHours(); saveMaintenance(); saveDegradation();
      triggerBatteryOff();
    }
  }
  if (now - lastDisplayMs >= displayRefreshIntervalMs()) {
    lastDisplayMs = now;
    drawDisplay(false);
  }
}
