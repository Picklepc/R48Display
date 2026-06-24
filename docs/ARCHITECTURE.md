# R48Display Architecture

R48Display is meant to stay stable first and expandable second. The release
firmware focuses on local battery telemetry for large lithium systems: mowers,
golf carts, trailers/RVs, portable power stations, marine/trolling packs,
utility carts, off-grid cabinets, and custom builds.

The core firmware should not assume one vehicle or one brand. It should expose a
normalized battery model, let the user choose a usage category, and adapt labels,
thresholds, and themes from that category.

## Current Modules

- `src/main.cpp`: boot orchestration, settings, Wi-Fi mode control, web routes,
  runtime counters, touch/buttons, usage categories, theme selection, and the
  active BMS loop.
- `src/DisplayUi.cpp` / `include/DisplayUi.h`: the four LCD pages only:
  dashboard, load/charge monitor, clock, and status. Theme colors are supplied
  through a snapshot so the LVGL display matches the selected web theme.
- `src/WebPages.cpp` / `include/WebPages.h`: embedded web page bodies and the
  small browser script. No SD card assets are required.
- `src/MicDetector.cpp` / `include/MicDetector.h`: optional audio assist for
  mower work detection. It must remain optional and category-gated.
- `include/BoardConfig.h`: board pinout for the Waveshare 1.85-inch ESP32-S3
  touchscreen variants.
- `include/FeatureModule.h`: small interface for optional modules once they are
  ready to be promoted into the firmware.

## Usage Category Rule

Usage categories provide user-facing context, not separate firmware forks.

Each category may define:

- Display labels for activity and high-load/work states.
- A default color theme.
- Whether mower-only audio assist is relevant.

The user can override the theme independently. Battery telemetry, BMS profiles,
OTA, Wi-Fi, and display pages should keep working the same way across all
categories.

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

## BMS Protocol Rule

Add each new BMS protocol as a profile, not as scattered conditionals. A profile
must define BLE discovery, GATT characteristics, authentication, polling, frame
reassembly, and parsing boundaries.

The web UI can expose all parsed values. The LCD should stay limited to stable
summary data that fits a 360 x 360 display: SOC, voltage, load/charge, runtime,
cell spread, link state, and actionable fault/status text.
