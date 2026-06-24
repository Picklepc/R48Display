# R48Display Firmware Audit

Audit date: 2026-06-24

Scope note: this audit applies to R48Display as a local large-pack lithium
monitor for mowers, golf carts, trailers/RVs, power stations, marine packs,
utility carts, off-grid systems, and custom builds. Older internal setting names
such as `mower_model` and `mower_run_amps` remain compatibility keys, but the
user-facing project should use usage categories and generic activity/work
language.

Verification performed:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run -e waveshare_esp32_s3_touch_lcd_1_85
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" check -e waveshare_esp32_s3_touch_lcd_1_85 --skip-packages
```

Result: build passed. Program size was about 1.59 MB flash and 176 KB RAM. `platformio check` passed; most findings were dependency noise. Local source findings are listed below.

## Phase 1: PlatformIO Environment and Build Configuration

Current state:

- `platformio.ini` uses the correct Waveshare ESP32-S3 Touch LCD 1.85 board and a custom partition CSV.
- `upload_speed = 921600` is fast but can be flaky on some USB cables/hubs. Keep it for maintainer use; use `460800` as a public default if testers report upload failures.
- `CORE_DEBUG_LEVEL=1` is useful for test firmware. Production builds should use `0` unless you want serial diagnostics.
- Add monitor filters so crash traces are usable by testers.
- OTA auth is hard-coded as `r48display` in `platformio.ini`. That is not a secret, but it is a weak default. Prefer an environment variable for maintainer OTA uploads.

Drop-in `platformio.ini` production baseline:

```ini
[env:waveshare_esp32_s3_touch_lcd_1_85]
platform = espressif32@7.0.1
board = waveshare_esp32_s3_touch_lcd_1_85
framework = arduino

monitor_speed = 115200
monitor_filters = time, colorize, esp32_exception_decoder
upload_speed = 460800

board_build.flash_mode = qio
board_build.flash_size = 16MB
board_upload.flash_size = 16MB
board_build.partitions = partitions.csv
board_build.arduino.memory_type = qio_opi
board_build.psram_type = opi

build_type = release
build_flags =
  -DARDUINO_USB_MODE=1
  -DARDUINO_USB_CDC_ON_BOOT=1
  -DBOARD_HAS_PSRAM
  -DCORE_DEBUG_LEVEL=0
  -DESP32QSPI_MAX_PIXELS_AT_ONCE=2048
  -DLV_CONF_INCLUDE_SIMPLE
  -Iinclude

lib_deps =
  moononournation/GFX Library for Arduino@1.6.0
  bblanchon/ArduinoJson@^7.4.2
  h2zero/NimBLE-Arduino@^2.5.0
  lvgl/lvgl@8.4.0

[env:waveshare_esp32_s3_touch_lcd_1_85_fast_usb]
extends = env:waveshare_esp32_s3_touch_lcd_1_85
upload_speed = 921600
build_flags =
  ${env:waveshare_esp32_s3_touch_lcd_1_85.build_flags}
  -DCORE_DEBUG_LEVEL=1

[env:waveshare_esp32_s3_touch_lcd_1_85_ota]
extends = env:waveshare_esp32_s3_touch_lcd_1_85
upload_protocol = espota
upload_port = r48display.local
upload_flags =
  --auth=${sysenv.R48DISPLAY_OTA_PASSWORD}
```

Partition review:

- Current layout is not minimal. It has `otadata` and two 5 MB OTA app slots, which is correct for OTA and leaves plenty of space for BLE/LVGL.
- Current SPIFFS is 0x5f0000 even though the stable firmware intentionally does not require a filesystem. That is harmless but oversized.
- Recommended release layout keeps large OTA slots and adds room for crash dump data/future small storage.

Drop-in `partitions.csv` if you want coredump/future storage instead of a huge SPIFFS area:

```csv
# Name,     Type, SubType, Offset,  Size,    Flags
nvs,        data, nvs,     0x9000,  0x5000,
otadata,    data, ota,     0xe000,  0x2000,
app0,       app,  ota_0,   0x10000, 0x600000,
app1,       app,  ota_1,   0x610000,0x600000,
coredump,   data, coredump,0xc10000,0x10000,
spiffs,     data, spiffs,  0xc20000,0x3e0000,
```

Crash handling:

- Keep the brownout detector enabled. Do not add code that disables it.
- In Arduino-core builds, most panic behavior is fixed by the core. The most useful production additions are reset-reason logging and exception decoder monitor filters.

Drop-in reset reason helper:

```cpp
#include <esp_system.h>

const char *resetReasonName(esp_reset_reason_t reason) {
  switch (reason) {
    case ESP_RST_POWERON: return "power-on";
    case ESP_RST_EXT: return "external";
    case ESP_RST_SW: return "software";
    case ESP_RST_PANIC: return "panic";
    case ESP_RST_INT_WDT: return "interrupt watchdog";
    case ESP_RST_TASK_WDT: return "task watchdog";
    case ESP_RST_WDT: return "other watchdog";
    case ESP_RST_DEEPSLEEP: return "deep sleep";
    case ESP_RST_BROWNOUT: return "brownout";
    case ESP_RST_SDIO: return "sdio";
    default: return "unknown";
  }
}

// In setup(), after Serial.begin():
Serial.printf("Reset reason: %s\n", resetReasonName(esp_reset_reason()));
```

## Phase 2: Dead Code and Optimization Audit

True unused/dead code:

- `src/main.cpp`: `scaleColor()` and `hexColor()` are unused.
- `src/main.cpp`: `otaReady` is set but never read.
- `src/DisplayUi.cpp`: `setLineColor()` is unused.
- `src/main.cpp`: `latestFrameHex`, `lastRxMs`, `lastConnectMs`, `lastWarningMs`, `found`, `balanceCurrentA`, `tempCount`, and `tempsC[]` are assigned but not exposed or otherwise consumed. Either expose them in JSON diagnostics or remove them.
- `extras/features/*` stubs are intentionally outside the build. They are not release dead code.
- Cppcheck reported `R48DisplayUi`, `R48Mic`, and `R48Web` public functions as unused. Those are false positives because they are used across translation units from `main.cpp`.

Drop-in removals:

```cpp
// Remove from src/main.cpp:
uint32_t scaleColor(uint32_t colorValue, float scale) { /* ... */ }
String hexColor(uint32_t colorValue) { /* ... */ }

// Remove from globals and setupOta():
bool otaReady = false;
otaReady = true;

// Remove from src/DisplayUi.cpp:
void setLineColor(lv_obj_t *obj, uint32_t fg) {
  if (obj) lv_obj_set_style_line_color(obj, color(fg), 0);
}
```

If you want to keep the currently assigned diagnostic fields, expose them instead of leaving them inert:

```cpp
// In addStatusJson(), inside the bms object:
b["found"] = bms.found;
b["last_rx_age"] = millisAge(bms.lastRxMs);
b["last_connect_age"] = millisAge(bms.lastConnectMs);
b["last_warning_age"] = millisAge(bms.lastWarningMs);
b["latest_frame_hex"] = bms.latestFrameHex;
b["balance_current_a"] = serialized(String(bms.balanceCurrentA, 2));
b["temp_count"] = bms.tempCount;
JsonArray temps = b["temps_c"].to<JsonArray>();
for (uint8_t i = 0; i < bms.tempCount && i < 16; ++i) {
  temps.add(serialized(String(bms.tempsC[i], 1)));
}
```

Memory and heap fragmentation risks:

- `String` use is heavy in web rendering and status JSON. This is acceptable for test firmware but should be reduced in hot paths and callback paths.
- `sendJson()` builds the full JSON into a temporary `String`. Stream it directly to the client.
- `sendContentPieces()` uses `substring()`, which allocates chunk strings. Write chunks directly.
- `MicDetector::loop()` allocates a 512-byte sample buffer on the stack every read. Use a static buffer.

Drop-in `sendJson()` replacement:

```cpp
void sendJson(JsonDocument &doc) {
  server.sendHeader("Connection", "close");
  server.setContentLength(measureJson(doc));
  server.send(200, "application/json", "");
  serializeJson(doc, server.client());
}
```

Drop-in `sendContentPieces()` replacement:

```cpp
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
```

Drop-in microphone stack reduction:

```cpp
void loop(bool packCurrentActive) {
  if (!state.enabled || !state.ready || millis() - state.lastReadMs < 250) return;

  static int16_t samples[256];
  size_t bytesRead = 0;
  const esp_err_t result = i2s_read(I2S_NUM_0, samples, sizeof(samples), &bytesRead, 0);
  // existing processing continues unchanged
}
```

## Phase 3: Stability and Crash Minimization

Concurrency/race condition risk:

- There are no custom FreeRTOS tasks and no custom ISRs in application code.
- `PIN_TOUCH_INT` is polled, not attached as an interrupt. That is good.
- The major race is the NimBLE notify callback. It currently calls `onNotify()`, reassembles frames, parses frames, and mutates global `bms` fields including `String` values. NimBLE callbacks may run on a BLE host task, not the Arduino loop. Avoid heap/String work in that callback.

Drop-in pattern: queue notification bytes in the callback and parse them from `BleBmsClient::loop()`:

```cpp
class BleBmsClient {
 public:
  void loop() {
    drainNotifyQueue();
    if (!bms.enabled) return;
    // existing loop body continues unchanged
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
  portMUX_TYPE notifyMux_ = portMUX_INITIALIZER_UNLOCKED;
  uint8_t notifyQueue_[NOTIFY_QUEUE_SIZE]{};
  size_t notifyHead_ = 0;
  size_t notifyTail_ = 0;
  volatile uint32_t notifyQueuedBytes_ = 0;
  volatile uint32_t notifyDroppedBytes_ = 0;

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

      bms.bytes += count;
      bms.lastRxMs = millis();
      appendRxBytes(local, count);
    }
  }

  void appendRxBytes(const uint8_t *data, size_t length) {
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
};
```

Missing error handling:

- `apiWifiScan()` does not handle negative scan results.
- `apiSettingsGet()` returns AP and OTA passwords. Do not echo secrets back to the browser.
- `configureClock()` makes WAN NTP requests to public servers whenever Wi-Fi connects. That conflicts with the LAN-only/locality goal. Make NTP optional or configurable to a LAN host.
- `initIoExpander()` ignores `tcaWrite()` failures.

Drop-in `apiWifiScan()` failure handling:

```cpp
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
```

Drop-in safer settings GET:

```cpp
void apiSettingsGet() {
  JsonDocument doc;
  doc["hostname"] = settings.hostname;
  doc["ap_ssid"] = apSsid;
  doc["ap_password_set"] = settings.apPassword.length() >= 8;
  doc["ota_password_set"] = settings.otaPassword.length() >= 8;
  doc["wifi_ssid"] = settings.wifiSsid;
  doc["wifi_password_set"] = !settings.wifiPassword.isEmpty();
  doc["bms_name"] = settings.bmsName;
  doc["bms_protocol"] = settings.bmsProtocol;
  doc["mower_model"] = settings.mowerModel;
  doc["usage_category"] = settings.usageCategory;
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
```

Drop-in LAN-only NTP settings:

```cpp
// Add to AppSettings:
bool ntpEnabled = false;
String ntpServer;

// In loadSettings():
settings.ntpEnabled = prefs.getBool("ntpOn", false);
settings.ntpServer = prefs.getString("ntpHost", "");

// In saveSettings():
prefs.putBool("ntpOn", settings.ntpEnabled);
prefs.putString("ntpHost", settings.ntpServer);

void configureClock() {
  setenv("TZ", settings.timezone.c_str(), 1);
  tzset();
  if (settings.ntpEnabled && !settings.ntpServer.isEmpty()) {
    configTime(0, 0, settings.ntpServer.c_str());
    ntpConfigured = true;
  } else {
    ntpConfigured = false;
  }
}
```

Drop-in IO expander init error status:

```cpp
bool initIoExpander() {
  bool ok = true;
  ok = tcaWrite(0x03, 0xF0) && ok;
  ok = tcaWrite(0x02, 0x00) && ok;
  tcaSetOutput(0x0C);
  delay(30);
  tcaSetOutput(0x0F);
  delay(150);
  if (!ok) Serial.println(F("IO expander not responding"));
  return ok;
}
```

## Phase 4: GitHub Release Checklist

Repository cleanliness:

- No real Wi-Fi credentials, API keys, or MAC addresses were found in the source/docs scan.
- `.gitignore` now ignores PlatformIO build output, VSCode settings, logs, environment files, and generated binary artifacts.
- This directory is not currently a Git repository, so no tracked/untracked status exists yet.
- `firmware/*.bin` exists locally. With `.gitignore`, those binaries should be distributed through GitHub Actions artifacts or Releases rather than committed as source.
- Missing standard public repo files: `LICENSE`, `CONTRIBUTING.md`, `SECURITY.md`.
- README is good for scope/build/first run, but should add a pinout table, flashing matrix, screenshots/photos, release artifact instructions, and privacy/security notes.

Drop-in `.gitignore` release baseline:

```gitignore
.pio/
.vscode/
*.code-workspace
*.log
__pycache__/
.env
.env.*
!.env.example
.DS_Store
Thumbs.db
*.bak
*.tmp
*.bin
*.elf
*.map
*.hex
*.uf2
*.factory
compile_commands.json
```

Drop-in README section to add:

```markdown
## Pinout

| Function | GPIO | Notes |
| --- | ---: | --- |
| LCD backlight | 5 | PWM brightness control |
| LCD QSPI CS/SCK/D0/D1/D2/D3 | 21/40/46/45/42/41 | ST77916 panel |
| Internal I2C SDA/SCL | 11/10 | IO expander |
| Touch SDA/SCL/INT | 1/3/4 | CST816 touch controller |
| BOOT button | 0 | Hold at boot for setup AP |
| Battery hold/key/ADC | 6/7/8 | Optional board battery circuit |
| I2S mic WS/SCK/SD | 2/15/39 | Optional mic assist, disabled by default |
```

Suggested release flow:

```markdown
## Release Artifacts

GitHub Actions builds the firmware on every push and uploads:

- `R48Display-vX.Y.Z.bin` for OTA/app-only flashing.
- `R48Display-vX.Y.Z-merged.bin` for full flash at offset `0x0`.
- `R48Display-vX.Y.Z.sha256` for integrity checks.

Do not commit generated `.bin` files. Attach them to GitHub Releases or use the workflow artifact.
```

## Phase 5: Refactoring Priority

Apply in this order:

1. Move BLE notification parsing out of the callback and into the main loop queue.
2. Stop returning passwords from `/api/settings`.
3. Make NTP opt-in and LAN-configurable.
4. Stream JSON and page chunks to reduce heap churn.
5. Expose or remove assigned-but-unused BMS diagnostic fields.
6. Remove `scaleColor()`, `hexColor()`, `setLineColor()`, and `otaReady`.
7. Add README pinout/release sections and a `LICENSE`.
