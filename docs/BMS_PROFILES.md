# BMS Profile Notes

R48Display supports battery monitoring through protocol profiles. A profile
describes how to discover a BMS, connect to its BLE service, request read-only
data, reassemble frames, and normalize telemetry into the common battery model.

The goal is broad compatibility across large lithium packs used in mowers, golf
carts, trailers/RVs, portable power stations, marine/trolling batteries, utility
carts, off-grid cabinets, and custom 36-60 V builds.

## Supported Profiles

### `humsienk_watt` — Humsienk / Hoperf WATT BLE
Stable profile validated locally. Read-only. Connects to service
`0000ae00-0000-1000-8000-00805f9b34fb`, authenticates with "HiLink", receives
notify on AE01, writes on AE02. Full protocol documented in
`docs/humsienk_ble_protocol.md`.

### `jbd_xiaoxiang_ff00` — JBD / Xiaoxiang BLE UART (FF00 variant)
Promoted stable. Service `0000ff00-0000-1000-8000-00805f9b34fb`, notify FF01,
write FF02. Polls `0x03` (basic info) and `0x04` (cell voltages). Read-only;
does not write settings or toggle MOSFETs.

### `jbd_xiaoxiang_ffe0` — JBD / Xiaoxiang BLE UART (FFE0 variant)
Promoted stable. Service `0000ffe0-0000-1000-8000-00805f9b34fb`, notify+write
FFE1. Same command set as the FF00 variant (0x03 / 0x04). Read-only.

### `jk_bms_ble` — JK BMS (Jikong) BLE
Implemented. Service `0000ffe0-0000-1000-8000-00805f9b34fb`, notify+write FFE1.
Uses the JK 20-byte request header `AA 55 90 EB [cmd] 00…` and expects responses
starting with `55 AA EB 90`. Parses `CELL_INFO (0x96)` for voltages, current,
temperature, SOC, Ah, and cycles; and `DEVICE_INFO (0x97)` for software version
and serial. Read-only.

### `daly_bms_ble` — Daly Smart BMS BLE
Implemented. Service `0000ae00-0000-1000-8000-00805f9b34fb`, notify AE01, write
AE02. Modbus-style framing: request `D2 03 [addr_hi] [addr_lo] [cnt_hi] [cnt_lo]
[crc_lo] [crc_hi]`; response `D2 03 [len] [data…] [crc_lo] [crc_hi]`. Reads
registers `0x00`–`0x51`: cell voltages, temperatures, pack voltage, signed
current, SOC, remaining Ah, cycle count, and balance state. Read-only.

Candidate future profiles are tracked in `docs/BLE_CONVERSION_RESEARCH.md`.
That file is research and backlog material, not a list of currently supported
protocols.

## Degradation Metric Availability by Profile

Fields marked **N** are provided natively by the BMS. Fields marked **C** are
computed locally from other native fields. Fields marked **—** are not available
from this protocol and fall back to zero.

| Field | humsienk_watt | jbd_ff00 | jbd_ffe0 | jk_bms_ble | daly_bms_ble |
|---|:---:|:---:|:---:|:---:|:---:|
| SOC | N | N | N | N | N |
| Pack voltage | N | N | N | N | N |
| Signed current | N | N | N | N | N |
| Remaining Ah | N | N | N | N | C (SOC÷100×total) |
| Total Ah | N | N | N | N | C |
| Cycle count | — | N | N | N | N |
| Cell voltages | N | N | N | N | N |
| Cell count | N | N | N | N | N |
| Cell delta (spread) | N | C | C | C | C/N† |
| MOS temperature | — | N | N | N | N (probe 0) |
| PCB temperature | — | N | N | N | N (probe 1) |
| Additional temps | — | N | N | N (probe T2) | N (probes 2+) |
| Balance state | N | N | N | N | N |
| Warning bits | N | N | N | — | — |
| Software version | — | N | N | N | — |
| Manufacturer/serial | — | N | N | N | — |
| Capacity fade % | local | local | local | local | local |
| Max cell spread (lifetime) | local | local | local | local | local |
| Min cell voltage (lifetime) | local | local | local | local | local |
| Max temp (lifetime) | local | local | local | local | local |
| Low voltage events | local | local | local | local | local |
| High current events | local | local | local | local | local |

† Daly natively reports cell delta in extended registers (`0x73`, registers
115-116); falls back to computed spread when those registers are absent.

All "local" fields are tracked by `updateDegradation()` in firmware regardless
of protocol, provided the underlying cell voltage, current, or temperature fields
are populated.

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
