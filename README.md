# R48Display

R48Display is a compact ESP32 battery display for large-capacity lithium packs:
mowers, golf carts, trailers and RVs, portable power stations, marine/trolling
packs, utility carts, off-grid cabinets, and custom 48 V builds.

The project started with Ryobi mower conversions, and that use remains important.
The firmware now treats the mower as one supported category instead of the only
category. Each install can choose its usage category, color scheme, activity
detection behavior, and BMS profile from the web settings.

## Supported Uses

- **Mowers**: battery SOC, load, charge, cell spread, runtime estimate, and
  optional mower work detection.
- **Golf carts**: drive/load status, hill or high-draw indication, and
  dashboard-friendly color themes.
- **Trailers and RVs**: pack health, inverter/load monitoring, charge status,
  and quiet utility-focused display styling.
- **Power stations**: output/load awareness, surge indication, and compact
  status display for portable or fixed power boxes.
- **Marine and trolling packs**: pack telemetry and high-load visibility without
  assuming a mower drivetrain.
- **Utility carts and custom packs**: generic activity/work thresholds for
  haulers, shop builds, off-grid systems, and other 36-60 V lithium projects.

## Stable Scope

- BLE battery monitoring for the Humsienk / Hoperf WATT profile.
- Test BLE battery monitoring for common JBD / Xiaoxiang BLE UART profiles.
- BLE scan and selectable BMS target by name/address plus matched protocol.
- Web dashboard and battery detail, including individual cell voltages.
- LCD dashboard, load/charge monitor, clock, and status page.
- Usage categories with selectable themes across the web UI and LVGL display.
- Selectable LCD rotation for alternate mounting angles.
- Wi-Fi STA mode for normal use.
- Temporary AP-only setup mode when no Wi-Fi credentials are saved or BOOT is
  held during startup.
- OTA update through PlatformIO or the embedded web updater.
- Optional onboard board-battery reading when the variant has that circuit.
- Optional microphone-assisted work detection for mower installs with the I2S
  microphone. It is disabled by default and is not required.

## Color Schemes

The same theme is applied to the browser UI and the LCD display:

- Chlorophyll Shift: green
- Redline Charge: red
- Violet Voltage: purple
- Blue Fairway: blue
- Orange Ignition: orange
- Parade Current: multicolored, bright, flamboyant, and deliberately loud
- Pixel Fairway: retro
- Modern Graphite: modern

Usage categories have default themes, but the theme can always be changed
independently.

## Hardware Target

- Waveshare ESP32-S3 Touch LCD 1.85.
- Works without SD card.
- Works without the onboard battery populated.
- Does not require gyro/IMU hardware.
- Leaves the bottom GPIO connector variant open for future accessories.

## Build

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run -e waveshare_esp32_s3_touch_lcd_1_85
```

To rebuild the distributable test firmware in `firmware/`:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\package_firmware.ps1
```

USB upload example:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run -e waveshare_esp32_s3_touch_lcd_1_85 -t upload --upload-port YOUR_PORT
```

OTA upload example after Wi-Fi is configured:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run -e waveshare_esp32_s3_touch_lcd_1_85_ota -t upload
```

## First Run

If no Wi-Fi credentials are saved, the board starts an AP-only setup network
named `R48Display-XXXXXX`. Connect to it, open the displayed IP, and save Wi-Fi,
usage, theme, and BMS settings.

After Wi-Fi is saved, the board uses STA-only mode. It does not keep AP+STA
running in the background.

## Activity And Work Detection

Activity detection is based on BMS discharge current. By default, current above
the activity threshold marks the system active, and current above the work/surge
threshold marks it under high load. The labels adapt by usage category: mowing,
climbing, hauling, inverter load, surge load, trolling, and so on.

Both detection layers can be disabled from settings. When disabled, the firmware
still shows pack telemetry and runtime estimates; it simply avoids forcing a
specific activity model onto the install.

The optional microphone assist is only for mower categories and only after BMS
current already shows activity. It cannot mark a system active from sound alone.

## Modular Development

Stable source files live in `src/` and `include/`. Optional feature experiments
start under `extras/features/`, outside the PlatformIO compile tree.

See:

- `docs/ARCHITECTURE.md`
- `docs/OPTIONAL_FEATURES.md`
- `docs/BMS_PROFILES.md`
- `docs/BLE_CONVERSION_RESEARCH.md`
- `PROMPT.md`
