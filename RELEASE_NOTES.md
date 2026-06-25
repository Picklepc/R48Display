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

# R48Display v1.0.0 — Release Notes

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

Flash `firmware.bin` at offset `0x10000` or use the merged binary at `0x0`:

```sh
esptool.py --chip esp32s3 --port YOUR_PORT write_flash 0x10000 firmware.bin
```

OTA update via PlatformIO after first flash:

```sh
pio run -e waveshare_esp32_s3_touch_lcd_1_85_ota -t upload
```
