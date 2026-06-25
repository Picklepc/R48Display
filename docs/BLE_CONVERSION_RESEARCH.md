# BLE Compatibility Research

Research date: 2026-06-24.

This file tracks common large-capacity lithium battery systems that are likely
to expose useful BLE BMS telemetry. Supported firmware profiles are documented
in `docs/BMS_PROFILES.md`; this file tracks what to test next.

R48Display started with Ryobi 48 V mower conversions, but the same monitor is
useful anywhere a large 36-60 V lithium pack needs local visibility: golf carts,
trailers/RVs, portable power stations, trolling/marine packs, utility carts,
off-grid cabinets, and custom DIY batteries.

Current firmware profiles (all read-only):

- `humsienk_watt`: validated against the local Humsienk / Hoperf WATT test
  battery.
- `jbd_xiaoxiang_ff00`: JBD / Xiaoxiang BLE UART modules advertising FF00 with
  FF01 notify and FF02 write.
- `jbd_xiaoxiang_ffe0`: JBD-style BLE UART modules exposing FFE0/FFE1.
- `jk_bms_ble`: JK BMS (Jikong) packs on FFE0/FFE1, using JK-specific 20-byte
  request frames. Parses CELL_INFO (0x96) and DEVICE_INFO (0x97) responses.
- `daly_bms_ble`: Daly Smart BMS BLE bridge. Modbus RTU framing over the WATT
  BLE service (AE00/AE01/AE02). Reads registers 0x00–0x51 for full telemetry.

All five profiles normalize the same telemetry fields. See
`docs/BMS_PROFILES.md` for the per-profile field availability table.

## Priority Use Cases

1. Golf carts
   - Large 48 V and 51.2 V LiFePO4 packs are common replacements for lead-acid
     carts from Club Car, EZGO, Yamaha, and similar platforms.
   - Many products advertise Bluetooth apps, SOC, current, cell status, and
     fault visibility. This is a high-value testing category because carts are
     used frequently and pack current changes quickly under hills and load.
   - Next work: capture BLE advertisements from common cart batteries and map
     which ones are JBD, Daly, JK, or vendor-specific.

2. Mowers
   - Ryobi 48 V conversions remain an important target because the project began
     there and many owners are replacing lead-acid packs with Bluetooth LiFePO4.
   - DC HOUSE, Enjoybot, Epoch, and DIY 16S packs appear repeatedly in owner
     reports and forum discussions.
   - Next work: keep collecting owner scan results, especially advertised name,
     address prefix, matched profile, cell count, pack voltage, current sign,
     and whether all cells populate.

3. Trailers and RVs
   - Trailer and RV users often care less about drive states and more about
     inverter draw, shore/solar charging, runtime estimate, and low-voltage
     warnings.
   - Many installs use 12 V batteries in series/parallel or a single 48 V pack
     feeding an inverter.
   - Next work: prioritize multi-BMS aggregation for four 12 V Bluetooth
     batteries and document how to configure pack topology safely.

4. Portable and fixed power stations
   - DIY power boxes and fixed shop/off-grid cabinets commonly use JBD, Daly,
     JK, or Overkill-style Bluetooth BMS modules.
   - These users need stable read-only telemetry, not vehicle-specific
     assumptions.
   - Next work: validate common DIY BMS modules and expose generic output/surge
     labels through the usage category model.

5. Marine and trolling packs
   - Trolling motor packs often use 36 V, 48 V, or 51.2 V lithium batteries with
     built-in Bluetooth monitoring.
   - The display should remain weather-conscious in language and avoid assuming
     a cart, mower, or inverter use case.
   - Next work: capture advertisements and app names from common marine battery
     vendors.

6. Utility carts, haulers, and custom packs
   - Utility vehicles and custom electric platforms may use the same batteries
     as carts and mowers but with different load expectations.
   - Next work: make threshold labels and runtime estimates useful without
     requiring a specific drivetrain model.

## Protocol Targets

- JBD / Xiaoxiang / Overkill-style BLE UART: implemented. Stable across FF00 and FFE0 variants.
- Daly: implemented. BLE bridge uses the WATT service UUIDs (AE00/AE01/AE02) with Modbus framing.
- JK (Jikong): implemented. Unique 20-byte request header over FFE0/FFE1.
- DC HOUSE: research in progress. Likely uses JBD-style protocol; app name "DCHOUSE". Needs BLE advertisement capture.
- Enjoybot: research in progress. Common in Ryobi mower conversions. Needs advertisement and frame capture.
- Vendor apps such as BAT-BMS and other private-label apps: likely to reuse JBD or JK framing; each needs at least one scan and data capture.

## Connection Notes

- A single 36-60 V pack with one BMS is the cleanest target because it exposes
  one SOC, one current reading, and one cell stack.
- The BLE scan page reports compatible profile IDs. Users should pick a scanned
  row instead of typing a display name manually; this saves both the target
  identity and the likely protocol.
- Four 12 V batteries in series should be modeled as multiple BMS devices plus
  one aggregate pack. Do not pretend they are one cell stack.
- Charger wiring, inverter wiring, cart controllers, mower lockouts, and marine
  wiring vary by install. Keep this project as a monitor first; do not add
  control assumptions to a battery profile.
- Do not hard-code brand names as default targets. Use profile UUID matching
  first, then saved name/address selection when a user wants a specific pack.

## Tester Report Template

Ask testers to report:

- Use case: mower, golf cart, trailer/RV, power station, marine/trolling,
  utility cart, off-grid, or custom.
- Battery brand, nominal voltage, and Ah.
- Mobile app name if known.
- BLE advertised name and whether the scan page reports a compatible profile.
- Pack voltage, SOC, signed current, cell count, and whether all cells populate.
- Whether current sign matches charge/discharge after toggling the setting.
- Any stale-data, reconnect, or pairing behavior.

## Sources

- Bluetooth SIG Battery Service overview:
  https://www.bluetooth.com/specifications/specs/bas-1-1/
- JBD BLE UART protocol notes from the Overkill Solar BMS project:
  https://github.com/FurTrader/OverkillSolarBMS/blob/master/Comm_Protocol_Documentation/BLE%20_bluetooth_protocol.md
- Generic Chinese/JBD BMS BLE protocol notes:
  https://shishir-dey.github.io/open_battery/
- DALY communication protocol overview:
  https://www.dalybms.com/news/daly-three-communication-protocols-explanation/
- Reddit r/Ryobi48vMowers conversion thread:
  https://www.reddit.com/r/Ryobi48vMowers/comments/1kh33be/step_by_step_instructions_for_conversion_to/
- DIY Solar Forum Ryobi conversion discussion:
  https://diysolarforum.com/threads/ryobi-48v-lawn-mower-conversion.87573/
- Peacock Software DC HOUSE Ryobi 480ex conversion writeup:
  https://peacocksoftware.com/blog/upgrading-my-ryobi-480ex-riding-mower-lifepo4
- Walmart DC HOUSE 48 V 50 Ah Bluetooth listing:
  https://www.walmart.com/ip/DC-HOUSE-48V-50Ah-LiFePO4-Golf-Cart-Battery-Built-in-100A-BMS-2C-Bluetooth-Low-Temp-Cut-Off-Suitable-Club-car-Yamaha-golf-carts-Ryobi-Mowers-Trolling/19501360727
- DCHOUSE iOS app listing:
  https://apps.apple.com/us/app/dchouse/id6502391925
- Enjoybot Ryobi mower battery page:
  https://enjoybot.com/pages/electric-lawn-mower-battery
- Enjoybot 48 V 100 Ah Ryobi review:
  https://enjoybot.com/blogs/lifepo4-battery-news/upgrading-your-ryobi-48v-mower-with-the-enjoybot-48v-100ah-lifepo4-battery-a-comprehensive-review
- Lawn Mower Forum RM300e lithium conversion thread:
  https://www.lawnmowerforum.com/threads/convert-ryobi-rm300e-to-lithium-battery.75347/
