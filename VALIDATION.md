# R48Display Release Validation Checklist

Run this checklist before tagging any versioned release. Each item should be
verified on a freshly flashed device. Record the firmware version, date, and
tester for each run.

---

## Validation Run Header

```
Firmware version : _______________
Flash date/method: _______________
Hardware revision: Waveshare ESP32-S3 Touch LCD 1.85
Tester           : _______________
Date             : _______________
Battery under test: ______________
Pass / Fail      : _______________
```

---

## Section 1 — Build Verification

- [ ] `platformio run -e waveshare_esp32_s3_touch_lcd_1_85` completes with
      no errors or warnings in local source files.
- [ ] Flash size is under 5.5 MB (leaves headroom for OTA slot).
- [ ] RAM usage (heap + static) is reported under 220 KB.
- [ ] `platformio check -e waveshare_esp32_s3_touch_lcd_1_85 --skip-packages`
      reports no findings in local `.cpp` or `.h` files.
- [ ] No credentials, Wi-Fi SSIDs, MAC addresses, or local file paths appear in
      committed source or markdown files.
- [ ] `.gitignore` covers `*.bin`, `*.elf`, `*.map`, `.pio/`, `.env`.

---

## Section 2 — Cold Boot

- [ ] Device boots to LCD splash / dashboard within 5 s of power-on.
- [ ] Serial monitor shows reset reason (e.g., `Reset reason: power-on`).
- [ ] No crash, panic, or brownout is printed within the first 30 s.
- [ ] If no Wi-Fi credentials are saved, device enters AP setup mode and
      the LCD shows the AP SSID and IP address.
- [ ] If Wi-Fi credentials are saved, device connects to STA, LCD shows IP,
      and no AP is broadcast.

---

## Section 3 — Wi-Fi Provisioning

- [ ] Hold BOOT at power-on → device starts temporary AP-only setup network.
- [ ] Connect a phone or laptop to `R48Display-XXXXXX`.
- [ ] Open the displayed IP in a browser. Settings page loads within 5 s.
- [ ] Enter a valid SSID and password, save, and reboot.
- [ ] After reboot, device connects to STA. AP is no longer visible.
- [ ] `r48display.local` resolves to the device IP on the same LAN.
- [ ] `/api/settings` GET does not return any password value — only boolean
      `_set` fields for passwords.

---

## Section 4 — LCD Pages

All four pages must be reachable by swiping left or right on the touchscreen.

**Dashboard (Page 0):**
- [ ] SOC ring displays correct percentage from BMS.
- [ ] Voltage reading matches BMS pack voltage.
- [ ] Activity label shows one of the four user-defined state labels.
- [ ] Runtime estimate (or charge ETA) is shown when current is non-zero.
- [ ] Theme colors are applied (background, ring, text match the selected
      theme).

**Power Monitor (Page 1):**
- [ ] Watts displayed correctly (voltage × |current|).
- [ ] Amps displayed with correct sign (+charge / -discharge or per setting).
- [ ] Hour counters show `hoursActive` and `hoursWorking`.
- [ ] ETA value updates as current changes.

**Clock (Page 2):**
- [ ] Analog clock hands display current time.
- [ ] Date and time text match the system clock.
- [ ] If NTP is disabled, clock shows time-since-boot or last-set time, and
      does not attempt to reach the internet.
- [ ] Wi-Fi indicator shows connected SSID or "No WiFi".

**Status (Page 3):**
- [ ] SSID, IP, BLE status, BMS name, firmware version, and uptime all shown.
- [ ] Board battery SOC and power source shown when hardware is populated.
- [ ] Overdue maintenance count shown as a badge when items are overdue.
      No badge shown when nothing is overdue.
- [ ] BLE mode shows current polling mode (FULL / CHARGING / IDLE /
      ACTIVE_SLOW / ACTIVE_MEDIUM / ACTIVE_FAST).

---

## Section 5 — BLE Battery Connection

- [ ] Open `/settings` in browser, click "Scan for BLE Devices". At least the
      target battery appears in the scan list within 10 s.
- [ ] Select battery and profile, save, reboot.
- [ ] After reboot, device connects to battery within 30 s. LCD shows BMS
      connected.
- [ ] `/api/status` JSON includes `bms.connected: true` and non-zero SOC.
- [ ] Cell voltage grid in web battery detail shows all cells.
- [ ] Disconnect battery power (or move out of BLE range) → LCD shows stale
      data indicator within 2× the polling interval.
- [ ] Reconnect → device reconnects automatically within 60 s.

---

## Section 6 — Web Dashboard

- [ ] `http://r48display.local/` loads within 3 s on a LAN connection.
- [ ] SOC gauge and voltage update every ~2.5 s without page reload.
- [ ] Battery detail page shows cell voltages with min/max highlighted.
- [ ] Status page shows all telemetry fields (voltage, current, SOC, temps,
      cycles, RSSI, BLE mode, firmware version, uptime, power source).
- [ ] Theme CSS is applied consistently between LCD and web (same palette).

---

## Section 7 — Settings Persistence

- [ ] Change theme in web settings. Save. Reboot. Confirm theme is restored.
- [ ] Change usage category. Save. Reboot. Confirm category and default labels
      are restored.
- [ ] Set custom state labels and thresholds. Save. Reboot. Confirm values
      persist.
- [ ] Set LCD timeout. Save. Reboot. Confirm screen sleeps after the set
      interval.
- [ ] Set NTP server. Enable NTP. Save. Reboot. Confirm clock syncs if on a
      LAN with a time server (or confirm it does not attempt NTP if disabled).

---

## Section 8 — Status Thresholds & Labels

- [ ] Set `chargeMinAmps`, `activityAmps`, `workAmps` via settings.
- [ ] Apply current at charging level → status shows `labelCharging`.
- [ ] Apply current below `activityAmps` → status shows `labelStandby`.
- [ ] Apply current between `activityAmps` and `workAmps` → status shows
      `labelActive`.
- [ ] Apply current above `workAmps` → status shows `labelWorking`.
- [ ] Web settings live preview updates the label display as threshold sliders
      are moved.

---

## Section 9 — Power Management (Internal Battery)

Requires a device with the onboard LiPo circuit populated.

- [ ] Power from USB only → `/api/status` reports `power_source: "usb"`.
      BLE uses FULL polling mode. Screen does not enforce battery-power timeout.
- [ ] Power from internal battery only → `/api/status` reports
      `power_source: "battery"`.
- [ ] Battery power + BMS is charging → BLE switches to CHARGING mode
      (poll every ~30 s). Charge ETA shown on LCD and web dashboard.
- [ ] Battery power + BMS fully charged + no load → BLE switches to IDLE
      mode. LCD Status page shows "BLE sleeping — next check in Xh Xm".
- [ ] Battery power + active discharge → polling mode matches SOC change rate
      (ACTIVE_SLOW / ACTIVE_MEDIUM / ACTIVE_FAST per spec in
      `docs/POWER_MANAGEMENT.md`).
- [ ] LCD timeout fires after configured interval when on battery power.
- [ ] Double-tap on screen wakes display from sleep without triggering a
      page swipe.
- [ ] CPU frequency drops to 80 MHz when display is sleeping in IDLE or
      ACTIVE_SLOW mode. Confirm with `getCpuFrequencyMhz()` log output.
- [ ] No-battery condition (ADC below threshold) → device logs shutdown
      warning and cuts power via hold pin.

---

## Section 10 — Hour Meter & Maintenance Tracker

- [ ] `/api/status` returns `hours.total`, `hours.standby`, `hours.active`,
      `hours.working` as floats in hours.
- [ ] Hour counters increment in the correct tier based on current state.
- [ ] Hours persist across reboot.
- [ ] Hours increment during the correct state only (standby counter does not
      increment during active/working).
- [ ] Web dashboard shows all four counters in the Hours card.
- [ ] `/api/maintenance` GET returns empty array on a clean device.
- [ ] Create a maintenance item via POST. Item appears in GET with correct
      `elapsed`, `remaining`, and `pct` values.
- [ ] `/api/maintenance/<id>/confirm` POST resets elapsed to zero and records
      current timestamp.
- [ ] `/api/maintenance/<id>` DELETE removes item from list.
- [ ] Overdue item (elapsed > interval) shows red indicator in web UI and
      increments the overdue badge on LCD Status page.
- [ ] Preset templates appear on a fresh device after first boot.
- [ ] Maintenance data persists across reboot.

---

## Section 11 — OTA Firmware Update

- [ ] Open `http://r48display.local/update` in browser.
- [ ] Upload a known-good `.bin`. Progress bar advances to 100 %.
- [ ] Device reboots automatically after successful upload.
- [ ] New firmware version is shown in `/api/status` and on LCD Status page.
- [ ] All settings from before the OTA update are preserved (NVS survives OTA).
- [ ] PlatformIO OTA upload (`-e waveshare_esp32_s3_touch_lcd_1_85_ota`)
      succeeds and device reboots to new firmware.

---

## Section 12 — Display Controls

- [ ] Swipe left → next LCD page. Swipe right → previous page.
- [ ] BOOT button short press (when provisioned) → toggle page or defined
      action. Long press → start provisioning AP.
- [ ] Brightness setting changes take effect on LCD immediately.
- [ ] Display rotation setting (0° / 90° / 180° / 270°) applied correctly
      after save and reboot.
- [ ] Display sleeps after timeout. Any touch or button wakes it. Double-tap
      wakes without consuming a swipe.

---

## Section 13 — Resilience & Edge Cases

- [ ] Boot with no BLE battery nearby. Device stays functional on all pages
      and shows "BMS disconnected" rather than crashing.
- [ ] Boot with no Wi-Fi credentials. AP setup mode starts without crashing.
- [ ] Rapid BLE reconnects (simulate signal dropout 5× in a row). Device
      reconnects each time without reboot.
- [ ] Stale BMS data (battery removed mid-session) → stale indicator appears
      on LCD and web. No crash.
- [ ] Large BLE frame (near 512-byte notification buffer) received without
      frame buffer overflow error.
- [ ] WiFi scan returns 0 results (no networks) → `/api/wifi/scan` returns
      empty list, not a server error.

---

## Validation Sign-Off

```
All sections passed : YES / NO
Failures noted     :
  Section ___ — Item ___ : _______________
  Section ___ — Item ___ : _______________

Tester signature   : _______________
Date               : _______________
Approved for tag   : YES / NO
```
