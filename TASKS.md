# R48Display Development Tasks

Task tracker for the R48Display project. Tasks are organized by milestone.
Complete each task in order within a milestone before advancing. Mark tasks done
with `[x]` and add a short note if the approach deviates from the description.

---

## Milestone 0 — Stability & Audit Fixes

These items are identified in `docs/FIRMWARE_AUDIT.md` and must be resolved
before any feature work lands. A build must pass after each item.

- [x] **M0-01** Move BLE notify callback to byte-queue only — parse frames from
      `BleBmsClient::loop()` on the Arduino task. Confirmed already implemented:
      `onNotify()` pushes bytes into `notifyQueue_[]` under `portENTER_CRITICAL`;
      all frame parsing runs in `loop()`.
- [x] **M0-02** Stop returning AP password and OTA password from
      `/api/settings`. Return boolean `_set` flags instead.
- [x] **M0-03** Make NTP opt-in. Added `ntpEnabled` and `ntpServer` settings.
      `configureClock()` only calls `configTime()` when NTP is explicitly enabled.
- [x] **M0-04** Remove dead functions: `scaleColor()`, `hexColor()`,
      `setLineColor()`, and `otaReady`. Confirmed absent via grep — already
      removed in a prior cleanup pass.
- [x] **M0-05** Expose or remove assigned-but-unused BMS diagnostic fields.
      All fields now in `/api/status` JSON: `frame_hex`, `last_rx_age`,
      `last_connect_age`, `last_warning_age`, `found`, `balance_current_a`,
      `temps` array (covers `tempCount`+`tempsC[]`), `balance_state`.
- [x] **M0-06** Replaced `sendJson()` with `reserve(measureJson)+serialize`
      version. `serializeJson(doc, server.client())` requires an lvalue — kept
      String path, pre-reserved to measured size to avoid reallocations.
- [x] **M0-07** Replaced `sendContentPieces()` to write chunks via
      `client.write()` directly without `substring()` allocations.
- [x] **M0-08** Move `MicDetector::loop()` sample buffer to a static local to
      avoid 512-byte stack allocation every 250 ms. `int16_t samples[256]`
      changed to `static int16_t samples[256]` in MicDetector.cpp.
- [x] **M0-09** Added `apiWifiScan()` negative-count error handling — returns
      503 with empty list on scan failure.
- [x] **M0-10** `initIoExpander()` now returns `bool` and checks write results.
- [x] **M0-11** Added reset-reason logging to `setup()` covering all
      `esp_reset_reason_t` cases.
- [x] **M0-12** Update `platformio.ini` to production baseline: upload_speed
      460800, `CORE_DEBUG_LEVEL=0`, `monitor_filters = esp32_exception_decoder,
      log2file`.
- [x] **M0-13** Added `CONTRIBUTING.md` (build, code style, PR process, BMS
      profile guide) and `SECURITY.md` (reporting, web/MQTT/NVS/BLE scope).
- [x] **M0-14** Pinout table in README.md (GPIO 5/21/40/46/45/42/41 LCD, 11/10 I2C, 1/3/4 touch, 0 boot, 6/7/8 battery, 2/15/39 mic).
- [x] **M0-15** Verify firmware binaries are not committed to git; confirmed
      `.gitignore` covers `*.bin`, `*.elf`, `*.map`, `*.hex`, `*.uf2`.

---

## Milestone 1 — Internal Battery & Power Management

Add structured support for the onboard LiPo battery. When running on internal
battery the device must reduce power consumption intelligently. Full spec is in
`docs/POWER_MANAGEMENT.md`.

### Power Source Detection

- [x] **M1-01** `PowerSource` enum (`External`, `Battery`, `Unknown`) implemented.
      Detected via USB CDC presence + battery ADC + rise-sample latch.
      Exposed in `/api/status` and LCD Status page PWR metric.
- [x] **M1-02** Expose `power_source` in `/api/status` JSON.
- [x] **M1-03** Add `powerSource` to the LCD Status page (page 3). Snapshot
      gets `powerSource` string; TOUCH metric replaced with PWR showing
      "external"/"battery"/"unknown".
- [x] **M1-04** Graceful shutdown when board battery ≤ 4% and < 3.4 V:
      saves settings/maintenance/degradation then drives PIN_BATTERY_HOLD LOW.
      Checked after every `updateScreenBattery()` call in the main loop.

### LCD Power Saving

- [x] **M1-05** Add `lcdTimeoutSec` setting (0 = never, default 300). Store in
      NVS. Expose in web settings under a "Power" section.
- [x] **M1-06** On USB/external power: keep the screen always on. Battery-only
      mode still applies `lcdTimeoutSec` and the hard battery cap below.
- [x] **M1-07** On battery power: enforce `lcdTimeoutSec`. If set to 0, apply
      a hard cap of 120 s to protect battery life regardless of user preference.
- [x] **M1-08** Implement double-tap wake via CST816 gesture or two quick touch
      events within 400 ms. Wake the display without consuming a navigation
      swipe.
- [x] **M1-09** When display is sleeping, reduce LVGL tick rate and skip
      `DisplayUi::draw()`. `tick(bool sleeping)` now uses 500 ms handler
      interval when sleeping vs 16 ms active. `drawDisplay()` already returned
      early on sleep; now `tick()` does too.

### BLE Adaptive Polling

Full decision tree is in `docs/POWER_MANAGEMENT.md`. Implement in this order:

- [x] **M1-10** Add `BlePollingMode` enum: `FULL`, `CHARGING`, `IDLE`,
      `ACTIVE_SLOW`, `ACTIVE_MEDIUM`, `ACTIVE_FAST`. Compute mode each loop
      cycle based on power source and current BMS state.
- [x] **M1-11** On `USB_POWER` or `UNKNOWN`: always use `FULL` mode (current
      5 s / 15 s / 30 s / 60 s polling intervals). No change from existing
      behavior.
- [x] **M1-12** On `BATTERY_POWER` + BMS is charging (current > 0.5 A charge):
      use `CHARGING` mode — poll every 30 s, show charge ETA on Power Monitor
      page and in web dashboard.
- [x] **M1-13** On `BATTERY_POWER` + BMS fully charged + no load (SOC ≥ 98 %,
      |current| < 0.3 A): use `IDLE` mode — disconnect BLE, wake to reconnect
      and poll 3–4 times per day (every ~6 h), immediately re-enter IDLE if
      conditions unchanged.
- [x] **M1-14** On `BATTERY_POWER` + active discharge: SOC rate mapped to
      polling mode. `updateSocRate()` computes `socRatePctPerHour` over rolling
      5-min window. `blePolicyCompute()` maps: >5%/h → ActiveFast (60s),
      >2%/h → ActiveMedium (3min), else → ActiveSlow (10min).
- [x] **M1-15** BLE sleep shows "BLE sleeping — next check in Xh Xm" on LCD
      Status (metric[8] via bmsStatus) and in web dashboard (`bms.status`
      field). `formatCountdown()` computes remaining ms from `lastAnalogPollMs
      + policy.analogMs`.
- [x] **M1-16** Add BLE disconnect/reconnect logic for battery power modes.
      Disconnecting BLE when IDLE saves ~10–15 mA. Reconnect on schedule or on
      display wake (user interaction = user wants fresh data).

### Additional Power Saving

- [x] **M1-17** Reduce CPU frequency to 80 MHz when display is sleeping and
      BLE polling mode is IDLE or ACTIVE_SLOW. `syncCpuFrequency()` called
      every loop checks both conditions and transitions 240↔80 MHz on change.
- [x] **M1-18** WiFi modem sleep: `syncWifiPowerSave()` promotes to
      `WIFI_PS_MAX_MODEM` when no web request in 60 s; drops back to
      `WIFI_PS_MIN_MODEM` on first request. `lastWebRequestMs` tracked in
      `sendPage()` and `apiStatus()`.
- [x] **M1-19** Cache the last known BMS snapshot to NVS (or PSRAM buffer) so
      the screen shows stale-but-labeled data on wake instead of a blank until
      BLE reconnects.
- [x] **M1-20** Add `batteryPowerMode` setting: `auto` (default), `always_full`,
      `always_save`. `always_full` disables all power saving even on battery.
      `always_save` forces maximum savings even on USB. Expose in web settings.

### Web Settings UI

- [x] **M1-21** Added "Power Management" settings card with live power source
      indicator (fetched from `/api/status` in `wireActions`), LCD timeout,
      battery power mode, and idle BLE wake hours. Fields moved from Time & NTP
      and System cards.

---

## Milestone 2 — Generalized Status Thresholds

Replace the hard-coded mower/activity model with user-configurable current
thresholds and user-defined state labels. This makes the status system useful
across all categories without mower-specific assumptions.

### State Model

States are determined by BMS current direction and magnitude:

| State | Condition | Example label (mower) | Example label (RV) |
|---|---|---|---|
| Charging | current > charge threshold | Recharging | Charging |
| Standby | \|current\| ≤ standby ceiling | Chilling | Standby |
| Active | standby ceiling < discharge ≤ work threshold | Driving | Running |
| Working | discharge > work threshold | Mowing | Peak Load |

- [x] **M2-01** Added four user-defined label settings: `labelCharging`,
      `labelStandby`, `labelActive`, `labelWorking`. Stored in NVS. Initialized
      from usage category defaults when empty; reset to new category defaults
      when usage category changes.
- [x] **M2-02** Added `chargeMinAmps` (default 0.5 A). `activityAmps` and
      `workAmps` map to existing `mowerRunAmps`/`bladesOnAmps` (NVS compat kept).
- [x] **M2-03** `mowerModeLabel()` now calls `stateLabel(activityState())`
      returning the user-defined label for the current four-state enum value.
      Category label fields (`activeLabel`, `workLabel`) are now only used as
      first-boot defaults.
- [x] **M2-04** Exposed `activity_state` (enum key string) and `activity_label`
      (user-defined string) in `/api/status` under the `vehicle` object.
- [x] **M2-05** Update LCD dashboard and power monitor pages to use the
      user-defined labels. Remove any remaining hard-coded state strings from
      `DisplayUi.cpp`.
- [x] **M2-06** Web dashboard `dash-mode` element shows `vehicle.mode` which is
      now the user-defined label. Status details table shows state key + label.
- [x] **M2-07** Added paired "Status Thresholds" settings for charge, standby,
      active, and working labels with their related amperage thresholds. NTP
      section added. `apiSettingsPost()` handles all new fields including
      category-change label reset.

---

## Milestone 3 — Hour Meter & Maintenance Tracker

Full spec in `docs/MAINTENANCE_TRACKER.md`. Replace the single runtime counter
with a multi-tier hour system and add a user-configurable maintenance reminder
system.

### Multi-Tier Hour Tracking

- [x] **M3-01** Replaced `runtimeSeconds` with four float counters
      (`hoursTotal`, `hoursStandby`, `hoursActive`, `hoursWorking`) stored in
      NVS as `hrsTotal`, `hrsStby`, `hrsAct`, `hrsWork`. Legacy `runtime` key
      is migrated into `hoursTotal` on first boot. `updateHours()` increments
      correct tier each loop based on `activityState()`. Saved every 60 s.
      Session active hours tracked in `sessionActiveHours` (resets on reboot).
- [x] **M3-02** All four counters exposed in `/api/status` under `hours` object
      alongside `session_active`.
- [x] **M3-03** Show `hoursActive` + `hoursWorking` on LCD Power Monitor page.
      Added HOURS metric row at y=324 showing "X.Xh act  Y.Yh work" via
      `Snapshot.hoursStr`.
- [x] **M3-04** Show hour counters in the web dashboard. Total, Active, and
      Working hours shown in the `dash-strip` section; JS populates from
      `hours.total`, `hours.active`, `hours.working` in `/api/status`.
- [x] **M3-05** Manual hour adjustment via `hours_total` (or legacy
      `runtime_hours`) POST parameter in `/api/settings`. Calls `setHoursTotal()`.

### BMS Degradation Metrics

- [x] **M3-06** `DegradationData` struct: maxCellSpreadMv, minCellVoltageV,
      maxTempC, lowVoltageEvents, highCurrentEvents. Persisted in NVS (5 keys:
      dMaxSpd/dMinCv/dMaxTc/dLvEvt/dHcEvt). cycleCount comes from BMS directly.
      `updateDegradation()` runs each new BMS analog frame.
- [x] **M3-07** `/api/status` → `degradation` object: cycle_count, capacity_
      fade_pct, max_cell_spread_mv, min_cell_voltage_v, max_temp_c,
      low_voltage_events, high_current_events, low_voltage_floor_v.
- [x] **M3-08** "Battery Health" card on web dashboard: capacity (% remaining /
      fade%), cycles, worst spread (lifetime), min cell (lifetime), max temp
      (lifetime), LV event count, HC event count.
- [x] **M3-09** `lowVoltageFloorV` setting (NVS key "lvFloorV", default 3.00 V,
      range 2.0–3.8 V). Exposed in Settings > Power Management. Low-voltage event
      increments once per discharge cycle when any cell drops below floor.
- [x] **M3-10** `docs/BMS_PROFILES.md` updated with full per-profile degradation
      availability table covering all five protocols (humsienk_watt, jbd_ff00,
      jbd_ffe0, jk_bms_ble, daly_bms_ble). Native vs. computed vs. local fields.

### Maintenance Reminder System

- [x] **M3-11** `MaintenanceItem` struct: id, name, MaintType enum
      (HOURS_TOTAL/ACTIVE/WORKING/DAYS), interval, lastResetTs, lastResetVal,
      notes. Max 20 items in `std::vector<MaintenanceItem> maintenanceItems`.
- [x] **M3-12** `loadMaintenance()` / `saveMaintenance()` via NVS key "maint"
      as a compact JSON array (~80 bytes/item × 20 ≈ 1.6 kB).
- [x] **M3-13** `GET /api/maintenance` — returns array with elapsed, remaining,
      pct, overdue fields per item.
- [x] **M3-14** `POST /api/maintenance` — creates (id=0) or updates (id>0).
      Validates name and interval. Returns saved item.
- [x] **M3-15** `POST /api/maintenance/confirm` — sets lastResetTs=now,
      lastResetVal=current hour/day counter. Returns updated item.
- [x] **M3-16** `POST /api/maintenance/delete` — removes item by id arg.
- [x] **M3-17** `/maintenance` web page: card-per-item with color progress bar
      (green/yellow/red), elapsed/remaining, Mark Done / Edit / Delete buttons,
      modal form for add/edit. Accessible from nav bar.
- [x] **M3-18** LCD Status page metric[8] shows "N maintenance item(s) due"
      when overdue count > 0; falls back to BMS error / status otherwise.
- [x] **M3-19** `initDefaultMaintenanceItems()` seeds three presets on first
      boot (empty list): blades @ 25 working-h, tires @ 100 active-h, annual
      @ 365 days. Saved immediately to NVS.

---

## Milestone 4 — Additional BMS Profiles

Expand BMS support beyond Humsienk/WATT and JBD test profiles.

- [x] **M4-01** Daly Smart BMS BLE profile implemented (`daly_bms_ble`).
      Modbus-style framing over AE00/AE01/AE02. Reads registers 0x00–0x51:
      cell voltages, temps, pack voltage, current, SOC, Ah, cycles, balance.
- [x] **M4-02** JK BMS (Jikong) profile implemented (`jk_bms_ble`).
      FFE0/FFE1 service. 20-byte request header `AA 55 90 EB`. Parses
      CELL_INFO (0x96) and DEVICE_INFO (0x97). Full telemetry coverage.
- [x] **M4-03** Promoted `jbd_xiaoxiang_ff00` and `jbd_xiaoxiang_ffe0` to stable.
      README updated: "BLE battery monitoring for common JBD / Xiaoxiang BLE UART
      profiles (FF00 and FFE0 variants)."
- [ ] **M4-04** Research DC House BLE protocol. Likely JBD-style; needs
      advertisement capture and frame log. App: "DCHOUSE". Documented as
      pending in `docs/BLE_CONVERSION_RESEARCH.md`.
- [ ] **M4-05** Research Enjoybot BLE protocol. Common in Ryobi mower
      conversions. Needs BLE advertisement and frame capture.
- [x] **M4-06** Per-profile degradation availability table added to
      `docs/BMS_PROFILES.md`. Covers all five protocols: native / computed /
      local breakdown for every telemetry field.
- [x] **M4-07** Tester report template present in `docs/BLE_CONVERSION_RESEARCH.md`
      (section "Tester Report Template" with 7 data points). New profiles
      should collect at least one report before marking stable.

---

## Milestone 5 — GitHub Stable Release (v1.0.0)

Gate criteria: all Milestone 0–3 tasks complete, at least one additional BMS
profile stable, validation checklist passes on a clean device.

- [x] **M5-01** Build 1.0.0 OTA-validated on device at 192.168.1.198 (2026-06-24).
      Device responded with firmware: "1.0.0", uptime 10s, all subsystems nominal.
- [x] **M5-02** `FIRMWARE_VERSION` set to `"1.0.0"` in `include/BoardConfig.h`.
- [ ] **M5-03** Tag `v1.0.0` in git.
- [x] **M5-04** `RELEASE_NOTES.md` written. Covers all new features (5 BMS profiles,
      MQTT/HA, degradation tracking, maintenance reminders, power management),
      known limitations, supported hardware, and flash instructions.
- [x] **M5-05** `.github/workflows/platformio.yml` updated. Runs on ubuntu-latest,
      triggers on push to main + pull requests + `v*` tags. Builds with pio,
      creates merged binary via esptool merge_bin, uploads firmware.bin +
      firmware-merged.bin + checksums.sha256 as artifacts on every build.
- [x] **M5-06** GitHub Release creation wired into the workflow. On `v*` tag push,
      `softprops/action-gh-release` creates a release with `RELEASE_NOTES.md`
      as the body and attaches the three build artifacts automatically.
- [x] **M5-07** `CONTRIBUTING.md`, `SECURITY.md`, and `LICENSE` all confirmed
      present and accurate (SECURITY.md includes MQTT + web + BLE + NVS scope).
- [x] **M5-08** README reviewed for public audience. Added JK/Daly to Stable Scope,
      MQTT section, simplified build instructions to standard `pio` commands.

---

## Milestone 6 — MQTT & Home Assistant Integration

Add optional MQTT publishing so battery state can be consumed by Home Assistant
and other MQTT-based automation systems. MQTT must be opt-in and must not affect
core BMS/display/OTA/WiFi behavior when disabled.

Library: `knolleary/PubSubClient` — compact, non-blocking, well-tested on ESP32.
Add to `platformio.ini` `lib_deps` once this milestone begins.

### MQTT Connection

- [x] **M6-01** MQTT settings in `AppSettings`: `mqttEnabled`, `mqttHost`,
      `mqttPort` (1883), `mqttUser`, `mqttPassword`, `mqttTopicPrefix`.
      All loaded/saved via NVS. GET returns `mqtt_password_set` bool, not plaintext.
- [x] **M6-02** `MqttClient` class in `include/MqttClient.h` + `src/MqttClient.cpp`
      wrapping PubSubClient. `begin()`, `loop()`, `publish()`, `publishRaw()`.
      Only connects when WiFi is up; never blocks loop.
- [x] **M6-03** Reconnect backoff: 5 s → 30 s → 60 s. Serial log on each attempt.
- [x] **M6-04** LWT `{prefix}/availability` → "offline" at will, "online" on connect.
      `setKeepAlive(60)` for automatic broker-side disconnect detection.
- [x] **M6-05** `/api/status` JSON includes `mqtt.enabled`, `mqtt.connected`,
      `mqtt.status`, `mqtt.broker`.

### Publishing Battery State

- [x] **M6-06** `publishMqttState()` sends JSON state to `{prefix}/state` on every
      new BMS analog frame. Fields: soc, voltage, current (signed), power, remaining_ah,
      cycles, cell_delta_mv, mos_temp_c, health_pct, status, status_label,
      hours_total/active/working, link.
- [x] **M6-07** `updateMqtt()` tracks previous activity state key; publishes on change.
- [x] **M6-08** Heartbeat every 60 s when BMS stale/disconnected; sets `link: stale/disconnected`.
- [x] **M6-09** `publishMqttCells()` sends cell voltage array to `{prefix}/cells`
      on new analog frame. Separate topic keeps main state topic small.
- [x] **M6-10** `/api/mqtt/publish-now` POST triggers immediate state+cells publish.

### Home Assistant Auto-Discovery

MQTT auto-discovery eliminates manual entity configuration in Home Assistant.
Each entity publishes a config message to `homeassistant/<component>/r48display_{id}/<entity>/config`.

- [x] **M6-11** `publishMqttDiscovery()` sends 15 retained discovery configs on connect.
      14 sensors (soc, voltage, current, power, remaining_ah, cycles, cell_delta,
      mos_temp, status, ble_link, health, hours_total/active/working) +
      1 binary_sensor (is_charging). All unit/icon/device_class set per table.

- [x] **M6-12** All discovery configs include `device` block:
      identifiers, name `"R48Display ({hostname})"`, model, manufacturer, sw_version.
      All 15 entities group under one device in HA.
      ```json
      "device": {
        "identifiers": ["r48display_{chip_id}"],
        "name": "R48Display ({hostname})",
        "model": "R48Display",
        "manufacturer": "R48Display",
        "sw_version": "{FIRMWARE_VERSION}"
      }
      ```
- [x] **M6-13** `/api/mqtt/rediscover` POST re-publishes all 15 discovery configs.

### Web Settings UI

- [x] **M6-14** "MQTT / Home Assistant" settings card in web settings page.
      Enable toggle, host, port (1883), username, password (placeholder), topic prefix.
      Buttons: Publish Now, Send HA Discovery, Test Connection.
      Status line shows result inline. Fields hidden when disabled.
- [x] **M6-15** `Snapshot.mqttStatus` fed to LCD Status page metric[8] priority chain.
      Shows "MQTT offline" or "MQTT connected" (suppressed when connected and no other alerts).

### MQTT Security Notes

- [x] **M6-16** `SECURITY.md` already documents MQTT plaintext (port 1883), local
      broker recommendation, and TLS as future work. Section confirmed present and accurate.

---

---

## Milestone 7 — v0.2.x Stability & Power Fixes

Post-release incremental fixes shipped in v0.2.1 through v0.2.3.

### Display & UX

- [x] **M7-01** LVGL battery page layout: CAPACITY removed, CELLS/TEMP moved to
      lower inward row, SPREAD widened to full center row. WATTS metric merged
      drain+charge into one centered indicator on dashboard.
- [x] **M7-02** Hours abbreviation corrected: "h work" → "h wrk" on LVGL battery
      page HOURS metric.
- [x] **M7-03** Status page BATT metric added: shows "on" / "off" for internal
      battery backup state (was PWR/SAVE).

### Web Dashboard & Settings

- [x] **M7-04** Mic RMS added to Device card above Heap Free. Shows live RMS when
      mic is enabled, "disabled" otherwise.
- [x] **M7-05** Settings > Power Management restructured: "Enable Internal Battery"
      toggle at top; LCD timeout, idle BLE wake, low-voltage floor, and low battery
      threshold collapse into `#batt-options` div shown only when battery is ON.
- [x] **M7-06** **Cut Battery Power** button added inside `#batt-options`. POST
      `/api/battery/off` saves NVS then pulses GPIO 7 KEY for 250 ms.
- [x] **M7-07** Configurable board battery low threshold (`boardBatteryLowPct`,
      NVS key `bbLowPct`, default 20 %). When board battery drops below this:
      low-power mode activates and hour counting pauses.
- [x] **M7-08** Install hours (baseline) can be updated independently from counted
      hours. `baselineChanged` guard in `apiSettingsPost()` skips `setHoursTotal()`
      when only the baseline field changes, preventing counted hours from zeroing.

### Shutdown & Power

- [x] **M7-09** `triggerBatteryOff()` replaces all bare `delay(80) +
      digitalWrite(PIN_BATTERY_HOLD, LOW)` sequences. Drives GPIO 6 (HOLD) LOW
      then pulses GPIO 7 (KEY) LOW for 250 ms — the KEY press immediately toggles
      the PMU regardless of load. GPIO 6 alone was insufficient because the IP5306
      only auto-shutoffs when load drops below ~100 mA, which never happens while
      WiFi and BLE are active.
- [x] **M7-10** `BatterySample.usbSofDetected` added: SOF-only detection (strict)
      separate from `usbCdcConnected` (broad inference). Shutdown block uses two
      independent paths: SOF-gone (immediate) and inference-gone (battery % drops
      below 80 % after being above it — slower fallback for DCP charger ports).
- [x] **M7-11** `setup()` calls `saveSettings()` after `loadSettings()` to seed
      any NVS keys that didn't exist yet (fields added in recent firmware updates
      such as `title`, `hoursBaseline`, `boardBatteryLowPct`) so they persist
      across the next reboot before the user visits the settings form.

### NVS Persistence Fixes

- [x] **M7-12** `loadDegradation()`, `saveDegradation()`, `loadMaintenance()`,
      `saveMaintenance()` each wrapped with their own `prefs.begin()`/`prefs.end()`
      pair. All four were silently failing because the preceding `loadSettings()` /
      `saveSettings()` call had already closed the NVS handle.

---

## Backlog (Post-v1.0.0)

These are desirable but explicitly out of scope for the initial stable release.
- [ ] Multi-BMS web UI aggregation (infrastructure exists; UI not built)
- [ ] Headless ESP32-C3 variant (BLE + web, no LCD)
- [ ] LAN-only push notifications (alert on low SOC, charge complete)
- [ ] MQTT TLS support (mbedTLS + certificate management)
- [ ] GPS/IMU optional feature promotion (separate branch)
- [ ] SD card trip log optional feature promotion (separate branch)
- [ ] GitHub Actions CI (build + static analysis on PR)
