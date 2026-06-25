# R48Display Architecture

R48Display is meant to stay stable first and expandable second. The release
firmware focuses on local battery telemetry for large lithium systems: mowers,
golf carts, trailers/RVs, portable power stations, marine/trolling packs,
utility carts, off-grid cabinets, and custom builds.

The core firmware should not assume one vehicle or one brand. It should expose a
normalized battery model, let the user choose a usage category, and adapt labels,
thresholds, and themes from that category.

---

## Current Modules

- `src/main.cpp` — boot orchestration, settings, Wi-Fi mode control, web routes,
  runtime counters, touch/buttons, usage categories, theme selection, power
  source detection, and the active BMS loop.
- `src/DisplayUi.cpp` / `include/DisplayUi.h` — the four LCD pages only:
  dashboard, load/charge monitor, clock, and status. Theme colors are supplied
  through a snapshot so the LVGL display matches the selected web theme.
- `src/WebPages.cpp` / `include/WebPages.h` — embedded web page bodies and the
  small browser script. No SD card assets are required.
- `src/MicDetector.cpp` / `include/MicDetector.h` — optional audio assist for
  mower work detection. It must remain optional and category-gated.
- `include/BoardConfig.h` — board pinout for the Waveshare 1.85-inch ESP32-S3
  touchscreen variants.
- `include/FeatureModule.h` — small interface for optional modules once they are
  ready to be promoted into the firmware.

---

## Usage Category Rule

Usage categories provide user-facing context, not separate firmware forks.

Each category may define:

- Display labels for activity and high-load/work states.
- A default color theme.
- Whether mower-only audio assist is relevant.

The user can override the theme independently. Battery telemetry, BMS profiles,
OTA, Wi-Fi, and display pages should keep working the same way across all
categories.

---

## Activity State Model

Four states are determined by BMS current direction and magnitude. All state
labels are user-configurable. Thresholds are stored in settings.

| State | Condition | Default label |
|---|---|---|
| Charging | current > chargeMinAmps | Recharging |
| Standby | \|current\| ≤ activityAmps | Standby |
| Active | activityAmps < discharge ≤ workAmps | Active |
| Working | discharge > workAmps | Working |

State is exposed in `/api/status` as `activity_state` (enum key) and
`activity_label` (user-defined string). LCD and web UI consume the label, not
the enum directly.

---

## Power Source & Power Management

See `docs/POWER_MANAGEMENT.md` for the full decision tree.

The firmware detects whether it is running from USB/external power or from the
internal LiPo battery (GPIO 8 ADC + VBUS sense). When on battery:

- BLE polling frequency is dynamically adjusted based on what the monitored
  battery is doing (charging, idle, or discharging at various rates).
- LCD timeout is enforced regardless of user preference (with a minimum floor).
- CPU frequency and WiFi modem-sleep are used to reduce idle current.
- The BLE connection may be dropped and re-established on a schedule to avoid
  holding a permanent connection when data updates are not needed frequently.

When on USB power, no power saving is applied by default (full BLE polling,
no enforced LCD timeout).

---

## Hour Meter & Maintenance

See `docs/MAINTENANCE_TRACKER.md` for the full specification.

Four operational hour counters replace the single `runtimeSeconds` field:

- `hoursTotal` — device powered on regardless of state
- `hoursStandby` — current below activity threshold
- `hoursActive` — current between activity and work thresholds
- `hoursWorking` — current above work threshold

A maintenance reminder system stores up to 20 user-defined items in NVS.
Each item tracks elapsed hours (or days, or BMS cycles) against a user-set
interval, with a confirmation workflow to log when maintenance is performed.

BMS degradation metrics (cycle count, capacity fade, lifetime extremes, event
counts) are tracked from BMS telemetry and local observation.

---

## BMS Protocol Rule

Add each new BMS protocol as a profile, not as scattered conditionals. A profile
must define BLE discovery, GATT characteristics, authentication, polling, frame
reassembly, and parsing boundaries.

The web UI can expose all parsed values. The LCD should stay limited to stable
summary data that fits a 360 × 360 display: SOC, voltage, load/charge, runtime,
cell spread, link state, and actionable fault/status text.

See `docs/BMS_PROFILES.md` for the current profile list and research workflow.

---

## Optional Feature Rule

Experimental features start outside `src/`, under `extras/features/<feature>/`.
PlatformIO does not compile those files by default. That keeps SD, navigation,
GPS, expanded audio, and other accessory work from destabilizing the battery
display.

When a feature is mature:

1. Keep its state and hardware ownership inside its own `.h/.cpp` pair.
2. Move it from `extras/features/<feature>/` into `src/features/<feature>/`.
3. Register it deliberately from `main.cpp`.
4. Add web/API/display integration only through narrow data snapshots.
5. Build and test the stable BMS/display flow before opening a PR.

---

## Document Index

| Document | Purpose |
|---|---|
| `TASKS.md` | Development task list organized by milestone |
| `VALIDATION.md` | Repeatable release validation checklist |
| `PROMPT.md` | Development philosophy and coding rules |
| `docs/ARCHITECTURE.md` | This file — module organization and design rules |
| `docs/POWER_MANAGEMENT.md` | Internal battery power saving spec and decision tree |
| `docs/MAINTENANCE_TRACKER.md` | Hour meter, degradation metrics, and reminder system |
| `docs/BMS_PROFILES.md` | Supported BMS protocols and research workflow |
| `docs/BLE_CONVERSION_RESEARCH.md` | Use case priorities and protocol research notes |
| `docs/OPTIONAL_FEATURES.md` | Optional feature promotion checklist |
| `docs/FIRMWARE_AUDIT.md` | Stability audit with drop-in fixes |
