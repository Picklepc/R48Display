# R48Display — Ryobi 48V Riding Mower Battery Indicator

A field guide for installing R48Display as a real-time battery monitor on Ryobi 48V riding mowers that have been converted to lithium packs with Bluetooth (BLE) battery management systems.

---

## Compatibility Notice

> **Only tested hardware:** Humsienk / Hoperf WATT 100Ah 48V LiFePO4 pack with BLE.
>
> If you use a different pack — JBD, JK BMS, Daly, or another brand — please [open an issue](https://github.com/Picklepc/R48Display/issues) and report whether it works. Protocol selection is configurable in the web settings; see [BMS protocol guide](BMS_PROFILES.md).

---

## What This Does

The Waveshare ESP32-S3 1.85" round LCD sits in your dash and shows:

- **State of charge** (SOC %) with animated ring gauge
- **Pack voltage** and **current mode** (charging / standby / active / mowing)
- **Runtime estimate** and **hour meter** (total, active, working hours)
- **Battery health**: cycle count, capacity fade, worst cell spread, max temp
- **Cell voltages** and **temperatures** on the battery page
- **MQTT / Home Assistant** integration for home automation

Everything is configured over Wi-Fi from a phone browser — no USB required after the first flash.

---

## Hardware Required

| Part | Notes |
|------|-------|
| [Waveshare ESP32-S3 Touch LCD 1.85"](https://www.waveshare.com/esp32-s3-touch-lcd-1.85.htm) | Round 360×360 display, built-in touch |
| Ryobi 48V lithium pack with BLE BMS | Humsienk 100Ah confirmed; others untested |
| 12V→5V buck converter (USB step-down) | Powers the ESP32 from the mower's 12V line located on the adjacent usb port |
| JST 2-pin LiPo (optional) | Onboard battery connector keeps the board alive when mower is off |
| ~2" round watch screen protector | Fits over the bezel; keeps dust out of the touch surface |
| Spray paint can cap (or 3D-printed back) | Houses the buck converter behind the display and sandwitches the screen to the ryobi panel |

---

## Wiring Overview

```
Ryobi 12V USB line ──► Buck converter (12V→5V) ──► ESP32-S3 USB-C
                                                            │
                                                     (optional JST LiPo
                                                      for standby power)
```

The Ryobi riding mower has a 12V accessory circuit that runs through the USB outlet. A small buck converter module (search "12V to 5V USB c step-down") plugs into that line and powers the ESP32 with clean regulated 5V. I purchased one with a right angle usbc, but the settings let you rotate the screen in 4 directions so that may not be required. When the mower key is off the buck converter loses power; the optional JST LiPo battery keeps the board alive for a few hours so the display stays functional during breaks.

---

## Enclosure: The Spray Can Cap Trick

The back of the round display has ~15mm of clearance in most dash cutouts. A standard aerosol spray paint cap (roughly 50–55mm inner diameter) fits neatly behind the screen and holds:

- The buck converter board
- The JST LiPo battery
- Any wire management

Drill a hole in the cap for the USB-C cable, hot-glue the cap to the back of the display, and tuck the electronics inside. The cap is easy to remove if you need to service the wiring.

---

## Optional: External Antenna

The ESP32-S3 on this board uses an integrated PCB antenna. Wi-Fi range with the onboard ceramic antenna can be marginal. The board has an IPEX/U.FL antenna connector and a small 0-ohm resistor that switches between the PCB trace and the connector.

**This is an advanced mod and requires soldering under magnification:**

- The resistor is 0402 size (approximately 1mm × 0.5mm) and can either be rotated or removed if you just solder across the pads.
- You must move it from the PCB-antenna pad to the IPEX connector pad
- An external 2.4GHz duck antenna + short IPEX pigtail then plugs into the connector

If you're not comfortable with 0402 SMD rework, the integrated antenna is usually sufficient for a typical yard. Only do this mod if you have documented SMD soldering experience.

---

## Sealing for Outdoor / Dusty Use

Mowing generates a lot of fine grass dust. After fitting the display:

1. Apply a **2-inch round watch screen protector** over the glass — search "2 inch round screen protector." Preferably 9h hardness. This protects the touch surface from scratches and absorbs light debris.
2. Run a thin bead of **silicone sealant** around the perimeter where the bezel meets the dash panel. Smooth with a wet finger. This prevents grass clippings from working their way behind the screen.
3. Leave the USB-C access point from the cap unsealed so you can reflash if needed, but a usb c dust cap is a good idea

---

## First Flash (USB)

1. Download the latest `firmware-merged.bin` from [Releases](https://github.com/Picklepc/R48Display/releases)
2. Use [ESPTool Web Flasher](https://espressif.github.io/esptool-js/) or `esptool.py`:
   ```
   esptool.py --chip esp32s3 write_flash 0x0 firmware-merged.bin
   ```
3. On first boot the display shows a setup QR code; connect to the `R48Display-Setup` AP and open `192.168.4.1` to enter your Wi-Fi credentials
4. After rebooting the device joins your network and all future updates can be done OTA from the web UI Settings → Firmware Update

---

## BLE Pairing to the Humsienk Pack

1. Open the web UI → **Settings → Battery Monitor (BMS)**
2. Set **Protocol** to `Humsienk / Hoperf WATT`
3. Click **Scan for BMS** — the pack advertises as something like `WATT` or the last digits of its MAC
4. Select your device from the list, click **Save**
5. The dashboard should start showing live SOC within 5–10 seconds

If the pack is not advertising, wake it by pressing its power button or connecting a charger briefly.

---

## Configuring for a Ryobi 48V Mower

Suggested settings (Settings page):

| Setting | Suggested value |
|---------|----------------|
| Pack / Vehicle Label | `Ryobi 48V 100Ah` |
| Usage Category | `Mower` |
| Typical Load Amps | `25–40 A` (varies by mowing conditions) |
| Active above (A) | `8 A` |
| Working / surge above (A) | `35 A` |
| Nominal Pack Ah | `100 Ah` |

Adjust Active and Working amps to match your actual mower draw — you can read live current on the Battery page while mowing to calibrate.

---

## Customizing with AI (Claude / Codex)

The full source is open and designed to be hackable. The easiest workflow:

1. Clone or download the repository
2. Open a conversation with [Claude Code](https://claude.ai/code) or your preferred AI assistant
3. Import the source files — the key ones are `src/main.cpp`, `src/DisplayUi.cpp`, `src/WebPages.cpp`
4. Ask it to make any changes you want: new metrics, different layouts, custom thresholds, additional BMS protocols
5. Build with PlatformIO and flash OTA

The codebase has no external dependencies beyond the libraries listed in `platformio.ini`. A capable AI model can read and modify the full codebase in a single context window, which makes iterative customization fast. Have fun with this, this mower is all about tinkering. A fun addition would be to add tesla ble tpms sensors to your mower and add a page that provides tire pressure to your gaugue. If anyone knows the guy that installed flipsky motor controllers on his, technically you could use the esp32 as a ble bridge to make the mower remote controlled. 

---

## Troubleshooting

| Symptom | Fix |
|---------|-----|
| BMS shows `scanning` forever | Wake the pack; check protocol matches your BMS brand |
| SOC shows `--` | BMS connected but no data yet — wait 15 s; try Reconnect |
| T5 / T6 temperatures show `--` | Normal — those probe slots are unpopulated on the pack |
| Wi-Fi drops after key-off | Normal if running on the 12V line only; add JST battery |
| Screen dim / no display | Check brightness slider in Appearance settings (Settings → Appearance) |
| OTA update fails | Device must be on your Wi-Fi; verify IP in System dashboard card |

---

## Links

- [Waveshare ESP32-S3 Touch LCD 1.85" product page](https://www.waveshare.com/esp32-s3-touch-lcd-1.85.htm)
- [Waveshare wiki / schematic](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.85)
- [R48Display releases](https://github.com/Picklepc/R48Display/releases)
- [BMS protocol reference](BMS_PROFILES.md)
- [Main README](../README.md)

---

*Only the Humsienk 100Ah 48V WATT pack has been tested. Please report compatibility results for other packs in [GitHub Issues](https://github.com/Picklepc/R48Display/issues) so this guide can be updated.*
