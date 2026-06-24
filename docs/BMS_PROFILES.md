# BMS Profile Notes

R48Display supports battery monitoring through protocol profiles. A profile
describes how to discover a BMS, connect to its BLE service, request read-only
data, reassemble frames, and normalize telemetry into the common battery model.

The goal is broad compatibility across large lithium packs used in mowers, golf
carts, trailers/RVs, portable power stations, marine/trolling batteries, utility
carts, off-grid cabinets, and custom 36-60 V builds.

Current firmware profiles:

- `humsienk_watt`: stable local profile based on the read-only WATT BLE protocol
  documented in `docs/humsienk_ble_protocol.md`.
- `jbd_xiaoxiang_ff00`: test profile for JBD / Xiaoxiang BLE UART modules that
  advertise service `0000ff00-0000-1000-8000-00805f9b34fb`, notify on FF01, and
  write on FF02.
- `jbd_xiaoxiang_ffe0`: test profile for JBD-style BLE UART modules that expose
  service `0000ffe0-0000-1000-8000-00805f9b34fb` and data characteristic FFE1.

The JBD test profiles read command `0x03` for basic info and command `0x04` for
cell voltages. They do not write settings, change Bluetooth module names, toggle
MOSFETs, or alter protection parameters.

Candidate future profiles are tracked in `docs/BLE_CONVERSION_RESEARCH.md`.
That file is research and backlog material, not a list of currently supported
protocols.

## Normalized Telemetry

Every profile should try to populate the same common fields:

- SOC
- Pack voltage
- Signed current
- Remaining, total, and design Ah
- Cycle count
- MOS/PCB temperatures where available
- Warning/fault bits
- Cell count and individual cell voltages
- RSSI, connected state, stale-data age, and last error

This normalization is what lets the same web UI and LCD pages work for a mower,
cart, trailer, power station, marine pack, or custom battery cabinet.

## Adding Another BMS

1. Create a branch named for the protocol, for example `feature/daly-ble`.
2. Keep parser and BLE session code isolated before touching the web UI.
3. Add a profile entry with service UUIDs, characteristic UUIDs, auth behavior,
   and parser selection.
4. Add a BLE scan identifier so users can select the device by name or address.
5. Preserve the common `BmsBleData` fields listed above.
6. If the BMS exposes extra fields, put them in the JSON API first. Only add LCD
   fields when they are useful on a 360 x 360 display.

Do not hard-code one user's battery name in the default release. The firmware
should connect to the first compatible advertised service unless the user saves a
target name or address.
