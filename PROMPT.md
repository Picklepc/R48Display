# R48Display Development Prompt

You are working in the `R48Display` project, a stable ESP32 display and web
monitor for large-capacity lithium battery systems.

Primary goal: keep the firmware a reliable BLE battery meter for the Waveshare
ESP32-S3 Touch LCD 1.85 while making it useful for mowers, golf carts, trailers
and RVs, portable power stations, marine/trolling packs, utility carts, off-grid
cabinets, and custom 48 V builds. The original Ryobi mower conversion audience
still matters, but mower-specific behavior must be selectable and optional, not
hard-coded into the whole project.

The firmware must boot and run with no SD card, no GPS, no IMU, no onboard
battery requirement, no required microphone, and no permanent AP+STA Wi-Fi mode.
Wi-Fi should run STA-only after provisioning. The setup AP should be temporary
and AP-only. The platform should remain able to grow toward headless ESP32-S3/C3
web-only builds without forcing LCD code into every variant.

Stable LCD pages:

- Dashboard
- Battery load and charge monitor
- Clock
- Text status page with SSID, IP, BMS, firmware, and useful diagnostics

Stable web features:

- Dashboard
- Battery detail with individual cells and equivalent telemetry where the BMS
  protocol supports it
- Usage category and color theme settings
- Activity/work detection thresholds that adapt labels by category
- BLE scan and BMS target selection
- Wi-Fi setup
- OTA update

Usage category rules:

- Supported categories include mowers, golf carts, trailers/RVs, power stations,
  marine/trolling packs, utility carts, off-grid/solar systems, and custom packs.
- Each category can provide default labels and a default theme.
- User-selected themes must override category defaults.
- Mower work detection and microphone assist must remain optional and disabled
  when not appropriate for the chosen category.
- Avoid hard-coded page text that assumes blades, decks, mowing, or a Ryobi
  mower unless it is inside a mower-specific label or compatibility note.

BMS protocol development routine:

- Prefer official protocol docs, vendor manuals, public SDKs, and owner-captured
  BLE logs before inspecting mobile apps.
- Use app inspection only to confirm protocol details and maximize
  compatibility.
- Extract advertised names, service UUIDs, characteristic UUIDs, notify/write
  directions, auth handshake, request frames, checksums, response layout,
  endianness, units, scaling, error codes, and timing.
- Bake common protocols into BMS profiles so untested batteries can connect when
  they use a known service/profile.
- Validate compatibility through user testing across golf carts, mowers,
  trailers/RVs, power stations, marine packs, and custom builds.
- Add each BMS as an isolated profile/parser module. Do not mix protocol parsing
  with display, web UI, Wi-Fi, or storage code.
- Preserve a normalized telemetry model across profiles: SOC, pack voltage,
  current, remaining Ah, total/design Ah, cycles, temperatures, warnings,
  individual cells, link status, RSSI, and stale-data age.
- Add conservative reconnect, timeout, and stale-data behavior. A missing or
  partially decoded frame should degrade status, not block the main loop.

Multi-battery support rules:

- Treat one 36-60 V pack with one BMS as the default stable case.
- For four 12 V Bluetooth batteries in series, model them as separate BMS
  devices plus one aggregate pack. Sum voltages, keep current handling explicit
  per protocol, and show weakest battery/cell as the primary risk indicator.
- For parallel packs, model each pack separately and aggregate only when the
  topology is configured by the user.
- Store target devices by address/profile/role instead of by display name alone.
  Names are useful for setup but not reliable identity.
- Limit simultaneous BLE work so Wi-Fi, display refresh, OTA, and touch remain
  responsive. Prefer short polling windows, connection reuse when stable, and
  clear backoff when a battery is unavailable.
- The web API should support aggregate pack data and per-device detail before
  the LCD tries to summarize multi-battery systems.

Development rules:

- Keep new features in their own `.h/.cpp` modules.
- Keep optional or experimental features outside `src/` until they are proven.
- Prepare the repo for GitHub testers. Keep personal data, MAC addresses,
  credentials, and local-only notes out of committed Markdown and config.
- Do not add SD card, navigation, GPS, expanded audio/microphone, IMU, file
  manager, or garage features to the stable firmware path unless explicitly
  promoted.
- Add new BMS protocols as clean profiles with isolated parsing code.
- Preserve the Humsienk/WATT profile and do not hard-code one battery name as
  the release default.
- Preserve network locality. The app should work standalone with or without a
  WAN connection.
- Leave room for LAN-only MQTT and Home Assistant reporting of battery status,
  low-battery alerts, charge-complete notifications, and fault states.
- Build before declaring work complete:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run -e waveshare_esp32_s3_touch_lcd_1_85
```
