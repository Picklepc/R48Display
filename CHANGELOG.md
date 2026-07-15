# Changelog

Full notes for each release live in [`docs/releases/`](docs/releases/) and on the
[GitHub Releases page](https://github.com/Picklepc/R48Display/releases). Each GitHub
release shows only that version's notes.

| Version | Summary |
|---|---|
| [v0.5.0a23](docs/releases/v0.5.0a23.md) | **Alpha (pre-release)** — on-device firmware updates re-enabled (user-initiated check only): the check tears BLE down to free RAM for the TLS handshake (no custom mbedTLS build), reports whether an update installs on-device (OTA) or is a major update needing USB, and saves hours before an OTA reboots. Needs a hardware test to confirm the handshake fits. |
| [v0.5.0a21](docs/releases/v0.5.0a21.md) | **Alpha (pre-release)** — full backup/restore (clone the whole NVS to migrate history to a replacement board; warns the file holds passwords in plaintext); hours/pay views reconciled (CSV driving no longer double-counts mowing and includes today; pay hours match earnings; pay CSV shows the active period); blocks app-only OTA across the 0.4.x/0.5.x bootloader boundary. |
| [v0.5.0a20](docs/releases/v0.5.0a20.md) | **Alpha (pre-release)** — reliable OTA: the ArduinoOTA/espota path now tears BLE down during the transfer (BLE + OTA were contending for radio/RAM and dropping the upload mid-stream on a live device). Install over USB; the fix helps subsequent over-the-air updates. |
| [v0.5.0a19](docs/releases/v0.5.0a19.md) | **Alpha (pre-release)** — boot audio intro plays right after boot instead of waiting for STA/BLE; restores prompt power-off on external-power loss (sharp pack-voltage-drop detection, ~8 s debounce) so it no longer runs for minutes on the onboard battery. Rolls up a16–a18 audio + display-panel work. |
| [v0.5.0a15](docs/releases/v0.5.0a15.md) | **Alpha (pre-release)** - fixes erased/fresh USB installer boot loops by packaging merged images with the same ESP32-S3 DIO boot header PlatformIO uses; adds selectable generic/Waveshare display-panel init modes for black-screen troubleshooting. |
| [v0.5.0a13](docs/releases/v0.5.0a13.md) | **Alpha (pre-release)** — steps back from the a12 BLE-worker-task boot loop to a11's proven coexistence-aware async BLE; adds the documented guard to init BLE only under WiFi `MIN_MODEM` (NimBLE #437 / ESP-IDF coexistence). Serial `[BLE]` scan counts drive the next proven step (passive vs active scan). |
| [v0.5.0a11](docs/releases/v0.5.0a11.md) | **Alpha (pre-release)** — keeps the 3.x migration but pins pioarduino to Arduino-ESP32 3.1.3 / ESP-IDF 5.3.2 to test BLE stability before reverting to the 0.4.x core. |
| [v0.5.0a10](docs/releases/v0.5.0a10.md) | **Alpha (pre-release)** — BLE recovery pass: delays BLE until STA/web are stable, uses passive scans, lets manual scan cancel auto scan, and pauses BLE after repeated scan timeouts instead of resetting the stack. |
| [v0.5.0a9](docs/releases/v0.5.0a9.md) | **Alpha (pre-release)** — completes the BLE coexistence pass by making the settings-page manual BLE scan bounded/non-blocking-safe too. |
| [v0.5.0a8](docs/releases/v0.5.0a8.md) | **Alpha (pre-release)** — BLE coexistence pass: non-blocking BMS scans, scan timeout recovery, earlier STA-mode NimBLE init, and manual-scan guarding. |
| [v0.5.0a7](docs/releases/v0.5.0a7.md) | **Alpha (pre-release)** — fixes the core-3 Wi-Fi drop: `WIFI_PS_MAX_MODEM` + BLE coexistence was dropping the STA link ~1 min after connecting (idle power-save), bouncing to the setup AP. Now holds `MIN_MODEM` on core 3, latches connection on the got-IP event, and logs Wi-Fi events over serial. Should also restore BLE (paused only while the AP is up). |
| [v0.5.0a6](docs/releases/v0.5.0a6.md) | **Alpha (pre-release)** — Wi-Fi stops reverting to the setup AP after a successful STA connect (latches on a DHCP lease; AP fallback now only for never-connected new credentials); adds USB-serial BLE diagnostics to trace why no devices are seen. |
| [v0.5.0a5](docs/releases/v0.5.0a5.md) | **Alpha (pre-release)** — setup AP compatibility pass for arduino-esp32 3.x: remove `softAPConfig`, isolate BLE, and lower setup AP TX power after start. |
| [v0.5.0a4](docs/releases/v0.5.0a4.md) | **Alpha (pre-release)** — setup AP recovery hardening: open fallback AP, explicit 192.168.4.1 config, and BLE paused while provisioning. |
| [v0.5.0a3](docs/releases/v0.5.0a3.md) | **Alpha (pre-release)** — installer erase prompt/data-preservation fix plus setup AP recovery and captive-portal redirect. |
| [v0.5.0a2](docs/releases/v0.5.0a2.md) | **Alpha (pre-release)** — first build on arduino-esp32 3.x / ESP-IDF 5.x; install over USB. Same features as v0.4.10; groundwork for on-device updates that fit the device's memory. |
| [v0.4.10](docs/releases/v0.4.10.md) | Web stability: stop the non-working on-device update checker (reclaims its 16 KB stack, ~doubling free RAM, and removes the heap churn that wedged the web server). |
| [v0.4.9](docs/releases/v0.4.9.md) | Web-server stability: stream JSON responses instead of one big String; pause dashboard polling when the tab is hidden. |
| [v0.4.8](docs/releases/v0.4.8.md) | Compact `/api/live` endpoint for the dashboard's recurring poll; static details table loads once — cuts recurring JSON overhead. |
| [v0.4.7](docs/releases/v0.4.7.md) | Update check no longer tears BLE down (was leaving the web UI unresponsive); on-device GitHub check confirmed memory-bound — use the USB installer. |
| [v0.4.6](docs/releases/v0.4.6.md) | Fix Maintenance page web-server crash by streaming large web assets and slimming heatmap/hour APIs. |
| [v0.4.5](docs/releases/v0.4.5.md) | Reduce Settings/Maintenance web load; fix current-day heatmap/pay timekeeping and custom activity-label consistency. |
| [v0.4.4](docs/releases/v0.4.4.md) | Fix BLE-teardown race that froze the unit / killed Wi-Fi after a failed update check; 30 s loop watchdog auto-reboots any hard freeze. |
| [v0.4.3](docs/releases/v0.4.3.md) | Update-check fix (dropped User-Agent header) + staged failure diagnostics (DNS/TCP/TLS/HTTP). |
| [v0.4.2](docs/releases/v0.4.2.md) | Maintenance page no longer wedges the web server; hour-counting pauses made visible & stale-proof; version pickers in installer + settings. |
| [v0.4.1](docs/releases/v0.4.1.md) | Update check falls back to freeing Bluetooth for the TLS handshake, fixing "could not reach GitHub" on memory-tight devices. |
| [v0.4.0](docs/releases/v0.4.0.md) | OTA auto-rollback on bad boot; wrong-file upload guard; version picker to install/roll back to any release. |
| [v0.3.7](docs/releases/v0.3.7.md) | Precise pay timekeeping (per-session log); split/back-date correction tool; live current-day hours. |
| [v0.3.6](docs/releases/v0.3.6.md) | One-click web USB installer; device self-updates over Wi-Fi from GitHub releases. |
| [v0.3.5](docs/releases/v0.3.5.md) | Pay Records; Maintenance-page crash fix; web-UI security hardening. |
| [v0.3.4](docs/releases/v0.3.4.md) | Weather display + 7-day forecast; ESP32-S3-Touch-LCD-1.85C support. |
| [v0.3.2](docs/releases/v0.3.2.md) | Animation/screensaver system; 15 themes reworked. |
| [v0.3.1](docs/releases/v0.3.1.md) | 256 KB NVS partition; settings-persistence fixes; touch navigation; AP privacy. |
| [v0.2.3](docs/releases/v0.2.3.md) | Battery power-off control; shutdown + NVS persistence fixes. |
| [v0.2.2](docs/releases/v0.2.2.md) | LVGL dashboard/battery/status refinements; temperature fixes. |
| [v0.2.1](docs/releases/v0.2.1.md) | LVGL layout; °F/°C + clock-format prefs; Ryobi 48V field guide. |
| [v0.2.0](docs/releases/v0.2.0.md) | Configurable display title; redesigned dashboard; settings reorg. |
| [v0.1.0](docs/releases/v0.1.0.md) | Initial release: BMS profiles, MQTT/HA, degradation, maintenance, hour tracking. |

**Upgrade note:** OTA from v0.3.0 or later is safe (unchanged partition table). Upgrading
from 0.2.x or earlier requires a one-time USB flash of the merged binary — see
[v0.3.1 notes](docs/releases/v0.3.1.md).
