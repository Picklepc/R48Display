# Contributing to R48Display

## Requirements

- PlatformIO Core or IDE
- ESP32-S3 target board (Waveshare ESP32-S3 Touch LCD 1.85)
- NimBLE-Arduino, LVGL 8.4, ArduinoJson 7.x (pulled automatically by `platformio.ini`)

## Building

```
pio run -e waveshare_esp32_s3_touch_lcd_1_85
```

First flash must be over USB. Subsequent updates can use OTA:

```
pio run -e waveshare_esp32_s3_touch_lcd_1_85_ota
```

## Code style

- C++17, Arduino framework, single-file firmware (`src/main.cpp`)
- No dynamic allocation in BLE callbacks — use the existing `notifyQueue_` ring buffer
- No `delay()` anywhere in the main sketch loop
- LVGL objects must only be created/destroyed from the Arduino (loop) task
- Keep `Snapshot` struct additions minimal; pass display data by value
- Match surrounding indentation (2-space, no tabs)
- Default to no comments; comment only non-obvious invariants or workarounds

## Pull requests

1. Fork and create a feature branch from `main`
2. One logical change per PR
3. Build must pass (`pio run`) before opening
4. Describe the BMS protocol or hardware you tested against in the PR body
5. Run the `VALIDATION.md` checklist for any change touching BMS parsing, display, or power management

## Adding a BMS profile

See `docs/BMS_PROFILES.md` for the research workflow and the `BMS_PROFILES` array in `src/main.cpp`. A new profile needs:
- Service and characteristic UUIDs
- Frame format documentation (or a reference to captured traffic)
- A parser function following the pattern of `parseJbdFrame()` / `parseWattFrame()`
- A tester validation report (template in `docs/BLE_CONVERSION_RESEARCH.md`)
