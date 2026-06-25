# Power Management

R48Display supports an optional onboard LiPo battery via the Waveshare board's
battery circuit (GPIO 6 hold, GPIO 7 key, GPIO 8 ADC). When powered by USB or
an external supply the device runs at full capability. When powered by the
internal battery it must aggressively conserve power to remain useful for
extended monitoring periods.

This document defines the power source detection logic, the BLE adaptive polling
decision tree, and the supplemental power saving measures. Task references map
to `TASKS.md` milestone M1.

---

## Power Source Detection

The Waveshare ESP32-S3 Touch LCD 1.85 battery circuit uses:

- **GPIO 6** — battery hold (keep power on; pull low to cut power)
- **GPIO 7** — battery key / power button sense
- **GPIO 8** — battery ADC (raw voltage divider reading of the LiPo cell)

USB VBUS detection is available via the ESP32-S3 USB PHY or by sensing whether
the 5 V rail reads above the battery LDO output voltage.

### Power Source States

```
enum PowerSource {
  POWER_USB,      // VBUS detected — USB cable, USB charger, or USB power bank
  POWER_BATTERY,  // No VBUS; running from internal LiPo
  POWER_UNKNOWN   // ADC circuit not populated; cannot determine source
}
```

Detection sequence (run once at boot, re-check every 30 s):

1. Read GPIO 8 ADC. If reading is below the minimum populated-battery threshold
   (≈ 3.0 V cell equivalent after voltage divider scaling), treat battery
   circuit as not populated → `POWER_UNKNOWN`.
2. Attempt VBUS sense. On ESP32-S3, `USB_SERIAL_JTAG.EP1_CONF.reg` or an ADC
   channel tied to the 5 V rail after a voltage divider can indicate VBUS.
   If board wiring does not expose VBUS, use the heuristic: if the ADC reads
   near full-charge voltage (≥ 4.1 V cell equivalent) and has been there since
   boot, VBUS is likely present.
3. If VBUS detected → `POWER_USB`.
4. If battery ADC reads a plausible mid-range LiPo voltage (3.2–4.15 V cell
   equivalent) and no VBUS → `POWER_BATTERY`.

When `POWER_BATTERY` is detected and the ADC reads below the emergency floor
(≈ 3.1 V cell equivalent):

1. Log the low-battery event.
2. Display "Battery critically low — shutting down" on LCD for 3 s.
3. Save all NVS state (hours, maintenance, settings).
4. Pull GPIO 6 LOW to cut power via the hold circuit.

---

## BLE Adaptive Polling Decision Tree

This tree runs every loop cycle when a power source is known. The result is a
`BlePollingMode` that overrides the default static polling intervals.

```
power source?
├── USB / UNKNOWN  →  FULL  (existing 5s/15s/30s/60s intervals)
└── BATTERY
    │
    ├── BMS charging? (current > +0.5 A)
    │   └── CHARGING
    │       • Analog poll: every 30 s
    │       • Cell poll: every 60 s
    │       • Show charge ETA on LCD Power page and web dashboard
    │
    ├── BMS fully charged + no load? (SOC ≥ 98 % AND |current| < 0.3 A)
    │   └── IDLE
    │       • Disconnect BLE
    │       • Wake every ~6 h (configurable, default 4× per day)
    │       • On wake: connect, poll once, update display, disconnect
    │       • On display wake (user touch): connect, poll once, disconnect
    │       • LCD Status shows "BLE sleeping — next check in Xh Xm"
    │
    └── BMS discharging
        │
        ├── Compute SOC change rate from 5-min rolling window
        │   (delta SOC % ÷ elapsed minutes × 60 = % per hour)
        │
        ├── Rate < 2 % / h  →  ACTIVE_SLOW
        │   • BLE on for poll, then disconnect
        │   • Poll interval: 10 min
        │
        ├── 2–5 % / h  →  ACTIVE_MEDIUM
        │   • BLE on for poll, then disconnect
        │   • Poll interval: 3 min
        │
        └── > 5 % / h  →  ACTIVE_FAST
            • Keep BLE connected
            • Analog poll: 60 s
            • Cell poll: 120 s
```

### Polling Interval Summary

| Mode | Analog | Cells | Warnings | BLE State |
|---|---|---|---|---|
| FULL | 5 s | 15 s | 30 s | Connected |
| CHARGING | 30 s | 60 s | 60 s | Connected |
| IDLE | — | — | — | Disconnected (wake on schedule) |
| ACTIVE_SLOW | 10 min | 20 min | 20 min | Connect → poll → disconnect |
| ACTIVE_MEDIUM | 3 min | 6 min | 6 min | Connect → poll → disconnect |
| ACTIVE_FAST | 60 s | 120 s | 120 s | Connected |

### Charge ETA Calculation

When `CHARGING` mode is active and current is available:

```
ahRemaining = nominalPackAh - (SOC / 100 * nominalPackAh)
ahToFull    = ahRemaining
etaHours    = ahToFull / chargeCurrentA
```

Display as "~Xh Ym to full" on LCD Power Monitor and in web dashboard charge
status card. Update every polling cycle.

### SOC Change Rate Calculation

Maintain a ring buffer of (timestamp, SOC) samples with a 5-minute window.
On each BLE poll update:

```
rate_pct_per_hour = (soc_oldest - soc_newest) / elapsed_minutes * 60
```

Where `soc_oldest` is the sample closest to 5 minutes ago and `soc_newest` is
the current reading. Use a minimum of 3 samples before changing polling mode to
avoid transient spikes causing premature mode switches.

Apply hysteresis: require the rate to cross a band threshold before switching
mode (e.g., must exceed 5.5 % / h to enter ACTIVE_FAST; must drop below
4.5 % / h to leave it). This prevents mode flapping at the boundary.

---

## LCD Power Saving

### Timeout Behavior

The `lcdTimeoutSec` setting (0 = never, default 300 s on USB, 120 s on battery)
controls when the backlight is cut.

On USB power: if `lcdTimeoutSec` is 0, the screen is always on. If non-zero,
the timeout is honored as set.

On battery power: `lcdTimeoutSec` is always applied. If the user sets it to 0,
a hard cap of 120 s is enforced regardless. This protects battery life.

Timeout reset events (restart the idle timer):
- Touch detected (CST816 interrupt or polling)
- BOOT button press
- BLE mode change (new poll results arrive)
- Web client activity

### Double-Tap Wake

CST816 supports gesture recognition. Configure it to detect a double-tap gesture
on the screen surface. On double-tap:

1. If display is sleeping → wake display, restart idle timer. Do not treat as a
   navigation swipe.
2. If display is awake → ignore (single tap continues to work for other
   gestures).

Alternatively, implement in software: track two touch-start events within a
400 ms window with no movement. Either approach is acceptable as long as a
regular single-swipe for page navigation is not disrupted.

### LVGL Rendering on Sleep

When the display is sleeping:
- Stop calling `DisplayUi::draw()`.
- Set LVGL tick rate to 0 (call `lv_tick_inc(0)` only, or gate the tick).
- Reduce `loop()` to BLE polling and NVS saves only.

On wake:
- Restore LVGL tick rate to 5 ms.
- Call `DisplayUi::draw()` with the latest snapshot.
- Force a full screen refresh on the first draw after wake to clear any
  artifacts.

---

## CPU & Radio Power Saving

### CPU Frequency

```
POWER_USB + screen on    → 240 MHz  (no change from default)
POWER_USB + screen off   → 160 MHz  (acceptable for web serving)
POWER_BATTERY + any mode → 80 MHz when screen is off
POWER_BATTERY + IDLE     → 80 MHz always (BLE disconnected, minimal work)
```

Use `setCpuFrequencyMhz(80)` / `setCpuFrequencyMhz(240)` with a brief delay
(10 ms) to allow peripherals to stabilize.

Note: 80 MHz may affect I2S microphone sampling if the microphone is enabled.
Disable microphone when in battery-power power saving modes (ACTIVE_SLOW, IDLE).

### WiFi Modem Sleep

```cpp
// Activate after 60 s of no web client connection:
esp_wifi_set_ps(WIFI_PS_MAX_MODEM);

// Restore on connection:
esp_wifi_set_ps(WIFI_PS_NONE);
```

In `WIFI_PS_MAX_MODEM`, the radio sleeps between DTIM beacon windows. Web
responses will be slightly slower (~100 ms latency) but power drops significantly
(~20–100 mA reduction depending on duty cycle).

Do not use modem sleep when OTA is in progress.

### BLE Disconnection Overhead

BLE connection idle current on ESP32 is approximately 10–20 mA. Disconnecting
BLE when not actively polling (IDLE, ACTIVE_SLOW, ACTIVE_MEDIUM) recovers this
budget. Reconnect latency is typically 1–5 s for known devices.

On reconnect, issue all pending polls immediately (analog + cells + warnings)
rather than waiting for the staggered polling schedule to advance.

---

## Stale Data Cache

When BLE disconnects for power saving, the last known BMS snapshot should remain
displayed rather than clearing to blank. The display shows:

- Last known SOC, voltage, and current values
- A "Last updated Xm ago" subtitle line in muted color
- The current BLE polling mode label

On battery power, this allows the user to glance at the display and see recent
data even when BLE is not actively connected.

Cache the last snapshot in a global `lastKnownBms` struct populated on each
successful BLE poll. The LCD draw function prefers live data when available and
falls back to the cache with a stale indicator.

---

## Power Management Settings

Add a "Power Management" section to the web settings page:

| Setting | Type | Default | Description |
|---|---|---|---|
| `powerSource` | read-only | — | Detected power source (USB / Battery / Unknown) |
| `batteryPowerMode` | enum | `auto` | `auto`, `always_full`, `always_save` |
| `lcdTimeoutSec` | integer | 300 (USB), 120 (battery) | LCD sleep timeout in seconds (0 = use hard cap on battery) |
| `idleWakeIntervalH` | float | 6.0 | Hours between BLE wake cycles in IDLE mode |
| `ntpEnabled` | bool | false | Enable NTP time sync |
| `ntpServer` | string | "" | LAN NTP server hostname or IP |

`batteryPowerMode`:
- `auto` — decision tree applies as described above
- `always_full` — use FULL polling regardless of power source (useful for
  testing or when plugged into a power supply without USB VBUS sense)
- `always_save` — use maximum power saving even on USB (useful for low-power
  supply scenarios)

---

## Power Budget Estimates

Reference values for the Waveshare ESP32-S3 Touch LCD 1.85 with a 500 mAh LiPo:

| State | Estimated Current | Runtime on 500 mAh |
|---|---|---|
| Full (BLE connected, LCD on, 240 MHz) | ~180 mA | ~2.8 h |
| BLE connected, LCD on, 80 MHz | ~130 mA | ~3.8 h |
| BLE disconnected, LCD on, 80 MHz | ~110 mA | ~4.5 h |
| BLE disconnected, LCD off, 80 MHz, WiFi modem sleep | ~40–60 mA | ~8–12 h |
| IDLE (BLE off, LCD off, 80 MHz, WiFi modem sleep) | ~30–40 mA | ~12–16 h |

These are rough estimates. Actual consumption depends on LCD brightness, WiFi
association interval, and ambient temperature. Measure with a USB power meter
during each mode to calibrate the estimates for the specific battery install.

A 1000 mAh LiPo roughly doubles all runtimes. The onboard battery circuit
supports up to the rated capacity of whatever cell is soldered in.
