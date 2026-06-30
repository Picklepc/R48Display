# R48Display v0.3.0 — Release Notes

## What's New in 0.3.0

### NVS Partition Expansion
- Default NVS storage is now 256 KB instead of the ESP-IDF default 20 KB. This gives room for settings, hour counters, cached BMS data, degradation metrics, maintenance items, and maintenance completion history.
- The enlarged NVS partition lives after the two OTA app slots at `0xa10000`. `otadata` remains at `0xe000` and `app0` remains at `0x10000` so Arduino/PlatformIO USB flashing keeps using the offsets it expects.
- `/api/status` includes NVS entry usage under `hardware.nvs` for field diagnostics.

### Fresh Install Required for the New NVS Layout
- Upgrading from 0.2.x to 0.3.0 requires a full USB/fresh flash so the new partition table is installed. OTA updates only replace the app partition; they cannot move or enlarge NVS.
- Because NVS moved from the old `0x9000` area to `0xa10000`, saved settings are not automatically migrated. Record Wi-Fi, BMS, MQTT, AP, and hour-meter settings before flashing, then re-enter them after first boot.
- Recommended recovery/fresh install flow: erase flash, then use PlatformIO USB upload or flash the merged release binary at offset `0x0`.

```sh
esptool.py --chip esp32s3 --port YOUR_PORT erase_flash
esptool.py --chip esp32s3 --port YOUR_PORT write_flash 0x0 R48Display-v0.3.0-merged.bin
```

### Firmware Packaging Fixes
- Local packaging and GitHub Actions now read `otadata` and `app0` offsets from `partitions.csv` when creating merged binaries.
- App-only `firmware.bin` remains for OTA and PlatformIO app uploads after the partition table is already installed. Use the merged binary for erased boards and recovery flashes.

### Maintenance History and Export
- Maintenance confirmations can now include completion notes.
- Each maintenance item keeps the 10 most recent completion records in NVS (`mh_<id>` keys).
- The Maintenance page includes a history drawer per item, CSV export, and an hour-meter breakdown bar.

### AP Mode Privacy
- AP password can be changed from Settings.
- The LCD setup status line can optionally hide AP SSID/password details and show only the setup URL.

---

# R48Display v0.2.3 — Release Notes

## What's New in 0.2.3

### Battery Power-Off (Manual Control)
- **Cut Battery Power** button added to Settings → Power Management (visible when Internal Battery is enabled)
- Saves all NVS data, then pulses GPIO 7 (PMU KEY pin) LOW for 250 ms to immediately toggle the battery off — device shuts down if on battery power, stays alive on USB with battery output cut

### Shutdown Fix — GPIO 7 KEY Pin
- Previous shutdown used GPIO 6 (HOLD pin), which only enables the PMU auto-shutoff when load drops below the minimum threshold; the ESP32 running WiFi and BLE never drops that low, so shutdown never triggered
- All shutdown paths now call `triggerBatteryOff()`, which drops HOLD (GPIO 6) and pulses KEY (GPIO 7) for an immediate PMU toggle

### USB Removal Detection
- `usbSofDetected` (SOF-only, strict) added to `BatterySample` and tracked separately from `usbCdcConnected` (broad inference)
- Shutdown uses two independent paths: SOF-gone (fires immediately when a USB host is removed) and inference-gone (fires when battery % drops below 80 % after being above it — slower fallback for DCP charger ports)

### NVS Persistence Fixes
- `loadDegradation()`, `saveDegradation()`, `loadMaintenance()`, `saveMaintenance()` were silently failing — each function called NVS get/put without a `prefs.begin()`, so the calls ran against a closed handle and returned defaults or wrote nothing
- `saveSettings()` is now called at the end of `setup()` after `loadSettings()` to seed any keys that don't exist yet (title, hoursBaseline, boardBatteryLowPct, and other fields added in recent releases), ensuring they persist across the next reboot even before the user visits the settings form

### Web — Power Management Restructure
- "Internal Battery" toggle controls the collapsible sub-options section; sub-options are hidden when battery is disabled
- Label updated from "Power Save" → "Enable Internal Battery" to reflect actual function
- Dashboard Device card "Power Save" label updated to "Batt Backup"

### Web — Mic RMS Dashboard
- Mic RMS value now appears on the Device card above Heap Free
- Shows live RMS when mic feature is enabled; shows "disabled" otherwise

### Hour Meter Fixes
- Working hours abbreviation corrected to "h wrk" on LVGL battery page
- Install hours (baseline) can now be updated without zeroing the counted portion — the form no longer incorrectly calls `setHoursTotal` when the baseline field changes
- Hours (active, working, baseline) now correctly persist across OTA updates via the NVS fix above

### Power Management — Configurable Battery Threshold
- New **Low Battery Threshold (%)** setting in Power Management — when the onboard LiPo drops below this level, low-power mode activates and hour counting pauses
- On startup from a depleted state, hours don't accumulate until the battery rises above the threshold (indicating a charging/powered state)

---

# R48Display v0.2.2 — Release Notes

## What's New in 0.2.2

### LVGL Dashboard
- Drain and charge watts combined into a single centered **WATTS** metric — they are mutually exclusive so one indicator is cleaner

### LVGL Battery Page
- **CAPACITY** metric removed from the battery page
- **CELLS** and **TEMP** moved to a lower inward row, mirroring the dashboard HOURS/LOAD layout and clearing the health arc
- **SPREAD** widened to a full center row above CELLS/TEMP

### LVGL Status Page
- **PWR** metric replaced with **SAVE** — shows power-save mode on/off instead of power source name
- When the device has no STA configured (AP-only mode), the bottom yellow status line cycles every 4 seconds through: Wi-Fi SSID, AP password, and login URL (`192.168.4.1`)

### Web — Temperature Fixes
- Temperature unit (°F/°C) now applied consistently across all web values: `bh-temp` (Battery Health max temp) was always showing °C regardless of the user's setting — now converts correctly
- Unpopulated temperature probe slots (T5/T6) previously showed `-- °F`; now show just `--`

### Web — Battery Page Gauge
- SOC gauge on the battery page now matches the dashboard gauge: same column proportions and ring size
- "SOC" label removed from both gauges — the percentage value already fills that space

---

# R48Display v0.2.1 — Release Notes

## What's New in 0.2.1

### LVGL Dashboard Layout
- ETA metric now spans the full top row, wider and easier to read
- LOAD and HOURS moved to a second row (HOURS left, LOAD right)
- New bottom row shows **Drain W** and **Charge W** (auto-scales W/kW); Charge watts turns blue (teal accent) when actively charging

### LVGL Battery Page
- CELLS metric shifted left, TEMP shifted right to clear the round health arc

### Temperature & Time Preferences
- **Temperature unit** (°F / °C) selector added to Appearance settings — applies to both web UI and LCD display
- **Clock format** (12h AM/PM / 24h military) added to Time & NTP settings — applies to LCD clock page and all time displays
- Temperatures reading exactly 0°C (unpopulated probe slots) now show `--` instead of converting to `32°F`

### Hour Meter Settings
- **Active Hours** and **Working Hours** are now editable in Hour Meter settings (previously read-only in the Details table)
- Each field includes an inline explanation of how it is calculated
- Edit to reset counters (set 0) or correct values after maintenance

### Ryobi 48V Field Guide
- New `docs/RYOBI_48V.md`: installation guide specifically for Ryobi 48V riding mowers with lithium BLE packs, covering wiring, spray-can-cap enclosure, external antenna mod, screen protector sealing, and AI-assisted customization

---

# R48Display v0.2.0 — Release Notes

## What's New in 0.2.0

### Configurable Display Title
- Added a new **Title** setting (separate from Hostname) in the System settings group
- `hostname` remains the mDNS/OTA/network identity; `title` is the display name shown in the web UI header and API
- If Title is left blank the UI derives it from the hostname (replacing dashes/underscores with spaces) — same behavior as before, now explicitly configurable

### Redesigned Web Dashboard
- System and Device status info (firmware version, uptime, IP, Wi-Fi, NTP, board battery, MQTT status) moved from a separate Status page to two cards at the bottom of the main dashboard
- `/status` route now redirects to `/` — no navigation disruption for existing bookmarks
- Status nav link removed; all relevant status metrics are visible inline on the dashboard

### Settings Reorganised
- Cards reordered for intuitive grouping: **Wi-Fi → System → Time/NTP → BMS → Appearance → Activity → Hour Meter → Power Management → MQTT/HA → Firmware**
- "Pack / Vehicle Label" (LCD identifier for the pack) kept in Appearance, clearly distinct from "Display Title" in System
- "Page Subtitle" renamed from "Display Subtitle" for clarity

---

# R48Display v0.1.0 — Release Notes

## What's New

### Four Supported BMS Profiles
- **Humsienk / Hoperf WATT** — validated on the project's original mower battery
- **JBD / Xiaoxiang FF00** — read-only BLE UART, FF01 notify / FF02 write
- **JBD / Xiaoxiang FFE0** — read-only BLE UART, FFE1 notify+write
- **JK BMS (Jikong)** — FFE0/FFE1, 20-byte request, cell info + device info frames
- **Daly Smart BMS** — Modbus-style BLE bridge, registers 0x00–0x51

All profiles are read-only. No settings are written, no MOSFETs are toggled.

### MQTT / Home Assistant Integration
Optional MQTT publishing (disabled by default). When enabled:
- Publishes battery state JSON to `{prefix}/state` on every new BMS reading
- Publishes cell voltage array to `{prefix}/cells`
- Sends 15 Home Assistant MQTT auto-discovery configs on connect (14 sensors + is_charging binary sensor)
- LWT availability topic (`{prefix}/availability`) for accurate HA availability tracking
- Reconnect with backoff: 5 s → 30 s → 60 s
- 60-second heartbeat when BMS is stale

### Battery Degradation Tracking
Lifetime worst-case metrics persisted in NVS regardless of BMS protocol:
- Maximum cell spread (mV)
- Minimum cell voltage (V)
- Maximum MOS temperature (°C)
- Low-voltage event count (cells dipping below configurable floor)
- High-current event count
- Capacity fade % (derived from cycle count via 0.05%/cycle model)

### Maintenance Reminder System
User-defined reminders tied to working hours, active hours, total hours, elapsed days, or BMS cycle count. Progress bars, overdue alerts on the LCD status page, and a full web UI at `/maintenance`.

### Multi-Tier Hour Tracking
Total, standby, active, and working hours. Configurable thresholds for each tier. Persisted in NVS across reboots and OTA updates.

### Power Management
When running from the onboard LiPo (not USB):
- Adaptive BLE polling: full rate while charging, throttled during active use, BLE disconnects when pack is idle and full
- LCD auto-sleep with configurable timeout
- CPU scaling to 80 MHz and WiFi modem sleep when idle
- Long-press (800 ms) on the display toggles power save mode instantly
- Graceful shutdown: saves all NVS state before cutting power via GPIO 6 hold pin

### Web Settings
- MQTT / Home Assistant card with broker config, action buttons, and inline status
- Power Management card: LCD timeout, idle BLE wake hours, low-voltage floor, power save toggle
- Maintenance page linked from navigation

## Known Limitations

- MQTT is plaintext (port 1883). Use a local broker or VPN for external access. TLS is a future addition.
- JK BMS and Daly support is based on published frame documentation; field offsets should be verified with hardware captures from real devices.
- Multi-BMS aggregation (multiple packs in series) is not yet implemented.
- DC HOUSE and Enjoybot private-label BLE protocols are under research; no firmware support yet.

## Supported Hardware

- Waveshare ESP32-S3 Touch LCD 1.85
- Onboard LiPo battery circuit (GPIO 6/7/8) — optional
- Onboard I2S microphone — optional, disabled by default

## Flashing

For USB/recovery flashing, use the merged binary at offset `0x0`. The app-only
`firmware.bin` is for OTA or PlatformIO uploads after the partition table is
already installed.

```sh
esptool.py --chip esp32s3 --port YOUR_PORT write_flash 0x0 R48Display-vX.Y.Z-merged.bin
```

OTA update via PlatformIO after first flash:

```sh
pio run -e waveshare_esp32_s3_touch_lcd_1_85_ota -t upload
```
