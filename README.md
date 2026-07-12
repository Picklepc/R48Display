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
- Weather display: current temperature and conditions on the clock face, 7-day forecast on the web dashboard (requires zip code in settings; uses Open-Meteo, no API key needed).
- Selectable LCD rotation for alternate mounting angles.
- Rotation-aware touchscreen swipe navigation for LCD pages.
- Wi-Fi STA mode for normal use.
- Temporary AP-only setup mode when no Wi-Fi credentials are saved or BOOT is
  held during startup.
- Automatic over-the-air self-update: the device checks GitHub for new releases
  and installs them itself over Wi-Fi. Manual OTA through PlatformIO or the
  embedded web uploader is still available.
- Optional MQTT publishing for Home Assistant and other automation systems.
  Opt-in; disabled by default. See [MQTT / Home Assistant](#mqtt--home-assistant).
- Optional onboard board-battery reading when the variant has that circuit.
- Optional microphone-assisted work detection for mower installs with the I2S
  microphone. Disabled by default and not required.

## Color Schemes

15 themes applied consistently across the browser UI and LCD display.

**8 utility themes** (always available):
- Chlorophyll Shift — green
- Redline Charge — red
- Violet Voltage — purple
- Blue Fairway — blue
- Orange Ignition — orange
- Pixel Fairway — retro
- Modern Graphite — modern
- Fabulous! — multicolored, bright, and deliberately loud

**7 seasonal themes** (calendar order):
- Be Mine — February
- 'Murica — July (includes animated US flag + fireworks)
- Starfield
- Ghouls & Goblins — October
- Turkey Trot — November
- Jingle All The Way — December
- Countdown — December 31

Each theme pairs with a default background animation. Animations can be toggled and overridden independently from the web settings.

Usage categories have default themes, but the theme can always be changed
independently.

## Hardware Target

- Waveshare ESP32-S3 Touch LCD 1.85 (primary target).
- Waveshare ESP32-S3 Touch LCD 1.85**C** (28-pin GPIO header variant) — fully compatible as of v0.3.4.
- Works without SD card.
- Works without the onboard battery populated.
- Does not require gyro/IMU hardware.

## Install

### Easiest — web installer (nothing to download)

Open **https://picklepc.github.io/R48Display/** in Google Chrome or Microsoft
Edge on a desktop computer, plug the board in with a **USB-C data cable** (not a
charge-only cable), and click **Install R48Display**. The page flashes the latest
release over USB and the device reboots into setup mode. Re-flashing keeps your
saved settings.

The installer uses Web Serial, which works in Chrome and Edge only — not Firefox
or Safari.

For erased boards or recovery flashes, use the web installer after the
fresh-install packaging fix is deployed, or use a regenerated merged binary.
Older raw release downloads can carry a QIO bootloader flash-mode header even
though PlatformIO direct USB flashing uses DIO on ESP32-S3; affected boards reset
before the app starts after a full erase.

### Updating

Once the device is on Wi-Fi it updates itself — no PC required. Open **Settings →
Firmware Update**, click **Check for updates**, then **Install** when a newer
release is available; the device downloads it from GitHub and reboots into the new
version. Leave **Automatically check for updates** enabled and it flags new
releases for you.

### Manual / recovery flashing

Download the merged binary from the
[latest release](https://github.com/Picklepc/R48Display/releases/latest) and flash
it at offset `0x0`:

```sh
esptool.py --chip esp32s3 --port YOUR_PORT write_flash 0x0 R48Display-vX.Y.Z-merged.bin
```

You can also use the [ESP32 Flash Download Tool](https://www.espressif.com/en/support/download/other-tools)
with the offset set to `0x0`, or build and flash from source (see [Build](#build)).

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

### v0.3.0 NVS Partition Change

Version 0.3.0 moves and enlarges the default NVS partition to 256 KB. This
requires a full USB/fresh flash from any 0.2.x or older install because OTA only
updates the app partition and cannot rewrite the partition table.

Recommended upgrade from 0.2.x:

1. Record Wi-Fi, BMS, MQTT, AP, and hour-meter settings. Existing NVS settings
   are not automatically migrated because the NVS partition moved.
2. Erase flash.
3. Flash over USB with PlatformIO, or flash the merged release binary at
   offset `0x0`.
4. Re-enter settings through the setup AP on first boot.

After 0.3.0 has been installed once with the new partition table, later app-only
OTA updates can be used normally.

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
| Active | Active | Discharge above activity threshold, including working |
| Working | Working | Discharge above work threshold; subset of active |

Category defaults are applied at first use. For example, the mower category
defaults to "Mowing" for the working state and "Driving" for the active state.
All labels are freely editable.

## Hour Tracking

Four hour counters are maintained in persistent storage and survive reboots and
OTA updates:

- **Total hours** — device powered on
- **Standby hours** — powered on, no significant draw
- **Active hours** — current draw above the active threshold, including working time
- **Working hours** — high current draw, always a subset of active hours

All four are shown in the web dashboard and available via the API.

## Maintenance Reminders

The Maintenance page (`/maintenance`) lets you set reminders tied to working
hours, active hours, total hours, elapsed days, or BMS cycle count. A progress
bar shows how close each item is to its interval. When due, items are flagged
in the web UI and a count appears on the LCD status page.

The page starts with two hour-meter bars: total displayed hours
(installation baseline plus tracked hours), then working / active / total
hours. Maintenance history rows can be expanded per item, edited for completion
date and notes, deleted, or exported as CSV.

The same page also stores machine/project notes: manufacturer, model number,
serial/VIN, manufacture date, gauge install date, battery model/install date,
custom user-defined fields, and freeform installation or part-number notes.

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

Firmware binaries are built by CI and attached to each
[GitHub Release](https://github.com/Picklepc/R48Display/releases). For most people
the [web installer](#install) or the device's built-in self-update is the easiest
path — both pull these same artifacts automatically. Per-version release notes are
in [`CHANGELOG.md`](CHANGELOG.md) and [`docs/releases/`](docs/releases/).

For a manual or recovery flash with `esptool.py`:

```sh
esptool.py --chip esp32s3 --port YOUR_PORT erase_flash
esptool.py --chip esp32s3 --port YOUR_PORT write_flash 0x0 R48Display-vX.Y.Z-merged.bin
```

For 0.3.0 and later, use the merged binary for erased boards, recovery flashes,
and any install that needs the partition table changed. The app-only
`firmware.bin` is for OTA or PlatformIO app uploads after the partition table is
already installed.

The web installer normalizes served merged ESP32-S3 images to the same DIO
bootloader flash-mode header used by PlatformIO direct USB upload, including the
stable default release. Release v0.5.0a15 and later also generate DIO merged
assets directly in CI. This matters for fresh-erased installs because the
bootloader must run before the app can repair or initialize NVS.

## Documentation

- [`CHANGELOG.md`](CHANGELOG.md) — per-version release notes index
- [`TASKS.md`](TASKS.md) — development task list by milestone
- [`VALIDATION.md`](VALIDATION.md) — release validation checklist
- [`PROMPT.md`](PROMPT.md) — development philosophy and rules
- [`docs/RYOBI_48V.md`](docs/RYOBI_48V.md) — Ryobi 48V riding mower installation guide
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
