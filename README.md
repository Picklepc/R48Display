# R48Display

R48Display is a compact ESP32 battery display for large-capacity lithium packs:
mowers, golf carts, trailers and RVs, portable power stations, marine/trolling
packs, utility carts, off-grid cabinets, and custom 48 V builds.

The project started with Ryobi mower conversions, and that use remains important.
The firmware now treats the mower as one supported category instead of the only
category. Each install can choose its usage category, color scheme, activity
detection behavior, and BMS profile from the web settings.

## Supported Uses

- **Mowers** — battery SOC, load, charge, cell spread, runtime estimate, and
  optional mower work detection.
- **Golf carts** — drive/load status, hill or high-draw indication, and
  dashboard-friendly color themes.
- **Trailers and RVs** — pack health, inverter/load monitoring, charge status,
  and quiet utility-focused display styling.
- **Power stations** — output/load awareness, surge indication, and compact
  status display for portable or fixed power boxes.
- **Marine and trolling packs** — pack telemetry and high-load visibility without
  assuming a mower drivetrain.
- **Utility carts and custom packs** — generic activity/work thresholds for
  haulers, shop builds, off-grid systems, and other 36–60 V lithium projects.

## Stable Scope

- BLE battery monitoring for the Humsienk / Hoperf WATT profile.
- BLE battery monitoring for common JBD / Xiaoxiang BLE UART profiles (FF00 and FFE0 variants).
- BLE battery monitoring for JK BMS (Jikong) packs via FFE0/FFE1.
- BLE battery monitoring for Daly Smart BMS packs via Modbus-style BLE bridge.
- BLE scan and selectable BMS target by name/address plus matched protocol.
- Web dashboard and battery detail, including individual cell voltages.
- LCD dashboard, load/charge monitor, clock, and status page.
- Usage categories with selectable themes across the web UI and LVGL display.
- User-configurable activity states with custom labels and current thresholds
  (charging, standby, active, working) for any use case.
- Multi-tier hour tracking (total, standby, active, working hours).
- Battery degradation metrics (cycle count, capacity fade, lifetime extremes).
- Maintenance reminder system with user-defined intervals and confirmation log.
- Internal battery power management: adaptive BLE polling, LCD timeout, CPU
  and WiFi power saving when running from the onboard LiPo.
- Selectable LCD rotation for alternate mounting angles.
- Wi-Fi STA mode for normal use.
- Temporary AP-only setup mode when no Wi-Fi credentials are saved or BOOT is
  held during startup.
- OTA update through PlatformIO or the embedded web updater.
- Optional MQTT publishing for Home Assistant and other automation systems.
  Opt-in; disabled by default. See [MQTT / Home Assistant](#mqtt--home-assistant).
- Optional onboard board-battery reading when the variant has that circuit.
- Optional microphone-assisted work detection for mower installs with the I2S
  microphone. Disabled by default and not required.

## Color Schemes

The same theme is applied to the browser UI and the LCD display:

- Chlorophyll Shift — green
- Redline Charge — red
- Violet Voltage — purple
- Blue Fairway — blue
- Orange Ignition — orange
- Fabulous! — multicolored, bright, flamboyant, and deliberately loud
- Pixel Fairway — retro
- Modern Graphite — modern

Usage categories have default themes, but the theme can always be changed
independently.

## Hardware Target

- Waveshare ESP32-S3 Touch LCD 1.85.
- Works without SD card.
- Works without the onboard battery populated.
- Does not require gyro/IMU hardware.
- Leaves the bottom GPIO connector variant open for future accessories.

## Pinout

| Function | GPIO | Notes |
|---|---:|---|
| LCD backlight | 5 | PWM brightness control |
| LCD QSPI CS/SCK/D0/D1/D2/D3 | 21/40/46/45/42/41 | ST77916 panel |
| Internal I2C SDA/SCL | 11/10 | IO expander (TCA9554) |
| Touch SDA/SCL/INT | 1/3/4 | CST816 touch controller |
| BOOT button | 0 | Hold at boot for setup AP |
| Battery hold/key/ADC | 6/7/8 | Optional onboard LiPo circuit |
| I2S mic WS/SCK/SD | 2/15/39 | Optional mic assist, disabled by default |

## Build

Install [PlatformIO](https://platformio.org/install/cli) (`pip install platformio`), then:

```sh
pio run -e waveshare_esp32_s3_touch_lcd_1_85
```

USB flash (first-time or if OTA is not yet configured):

```sh
pio run -e waveshare_esp32_s3_touch_lcd_1_85 -t upload --upload-port YOUR_PORT
```

OTA upload after Wi-Fi is configured (edit `upload_port` in `platformio.ini` to
your device IP):

```sh
pio run -e waveshare_esp32_s3_touch_lcd_1_85_ota -t upload
```

## First Run

If no Wi-Fi credentials are saved, the board starts an AP-only setup network
named `R48Display-XXXXXX`. Connect to it, open the displayed IP, and save Wi-Fi,
usage, theme, and BMS settings.

After Wi-Fi is saved, the board uses STA-only mode. It does not keep AP+STA
running in the background.

## Activity States & Labels

Current-based status is split into four configurable states. All labels and
thresholds are set from the web settings page and persist across reboots.

| State | Default label | Condition |
|---|---|---|
| Charging | Recharging | BMS current positive above charge threshold |
| Standby | Standby | Discharge below activity threshold |
| Active | Active | Discharge between activity and work thresholds |
| Working | Working | Discharge above work threshold |

Category defaults are applied at first use. For example, the mower category
defaults to "Mowing" for the working state and "Driving" for the active state.
All labels are freely editable.

## Hour Tracking

Four hour counters are maintained in persistent storage and survive reboots and
OTA updates:

- **Total hours** — device powered on
- **Standby hours** — powered on, no significant draw
- **Active hours** — mid-range current draw
- **Working hours** — high current draw

All four are shown in the web dashboard and available via the API.

## Maintenance Reminders

The Maintenance page (`/maintenance`) lets you set reminders tied to working
hours, active hours, total hours, elapsed days, or BMS cycle count. A progress
bar shows how close each item is to its interval. When due, items are flagged
in the web UI and a count appears on the LCD status page.

## MQTT / Home Assistant

Enable MQTT in **Settings → MQTT / Home Assistant**. Enter your broker host, port
(default 1883), and optional credentials. The topic prefix defaults to
`r48display/{hostname}`.

When enabled, the firmware:
- Publishes battery state to `{prefix}/state` on every new BMS reading and on
  activity state changes (charging ↔ standby ↔ active ↔ working).
- Publishes cell voltages to `{prefix}/cells` whenever cell data changes.
- Sends Home Assistant MQTT auto-discovery configs on connect so all entities
  appear automatically under a single device in HA.
- Publishes `online` / `offline` to `{prefix}/availability` for HA availability
  tracking.
- Sends a heartbeat every 60 s when the BMS is stale or disconnected.

Use **Publish Now** or **Send HA Discovery** in the settings page to trigger
manual publishes or re-register HA entities after a hostname change.

See [`SECURITY.md`](SECURITY.md) for MQTT security notes (plaintext by default;
use a local broker or VPN for external access).

## Internal Battery Power Saving

When running from the onboard LiPo (no USB power), the firmware adjusts BLE
polling frequency based on what the monitored battery is doing:

- **Charging** — more frequent updates, charge ETA displayed.
- **Idle (full, no load)** — BLE disconnects and wakes a few times per day.
- **Discharging** — polling rate scales with how fast the SOC is changing:
  slow draw → infrequent polls; fast draw → frequent polls.

The LCD sleeps after a configurable timeout when on battery power. Double-tap
the screen to wake it. CPU frequency and WiFi modem sleep are also managed
automatically.

See `docs/POWER_MANAGEMENT.md` for the full decision tree and power budget.

## Release Artifacts

Firmware binaries are built by CI and attached to GitHub Releases. Do not flash
`.bin` files from the `firmware/` directory in the repository directly — use the
release artifacts or build from source to ensure you have the latest version.

To flash the merged binary at offset `0x0` with `esptool.py`:

```sh
esptool.py --chip esp32s3 --port YOUR_PORT write_flash 0x0 R48Display-vX.Y.Z-merged.bin
```

## Documentation

- [`TASKS.md`](TASKS.md) — development task list by milestone
- [`VALIDATION.md`](VALIDATION.md) — release validation checklist
- [`PROMPT.md`](PROMPT.md) — development philosophy and rules
- [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) — module organization
- [`docs/POWER_MANAGEMENT.md`](docs/POWER_MANAGEMENT.md) — power saving spec
- [`docs/MAINTENANCE_TRACKER.md`](docs/MAINTENANCE_TRACKER.md) — hour meter and reminders
- [`docs/BMS_PROFILES.md`](docs/BMS_PROFILES.md) — supported BMS protocols
- [`docs/BLE_CONVERSION_RESEARCH.md`](docs/BLE_CONVERSION_RESEARCH.md) — protocol research
- [`docs/OPTIONAL_FEATURES.md`](docs/OPTIONAL_FEATURES.md) — optional feature promotion rules
- [`docs/FIRMWARE_AUDIT.md`](docs/FIRMWARE_AUDIT.md) — stability audit and fixes

## Modular Development

Stable source files live in `src/` and `include/`. Optional feature experiments
start under `extras/features/`, outside the PlatformIO compile tree.

See `docs/ARCHITECTURE.md` for the feature promotion checklist.
