# HumsiENK BLE Protocol Notes

Example target battery captured during development:

- Name: redacted battery advertisement
- Address: redacted BLE MAC address
- App package inspected: `uni.UNI3890CA7`, HumsiENK Smart BMS `1.2.0`

The working profile is the app's `WATT` protocol, not the JBD/Xiaoxiang UART-over-BLE protocol.

## GATT

- Service: `0000fff0-0000-1000-8000-00805f9b34fb`
- Notify/read: `0000fff1-0000-1000-8000-00805f9b34fb`
- Command write: `0000fff2-0000-1000-8000-00805f9b34fb`
- Auth write: `0000fffa-0000-1000-8000-00805f9b34fb`

The readable values of the HOPERF characteristics are ASCII `HOPERF`; they are not the battery data.

## Session

1. Connect.
2. Subscribe to notifications on `fff1`.
3. Write ASCII `HiLink` to `fffa`, with response.
4. Write read-only WATT frames to `fff2`, with response.
5. Reassemble notifications from `fff1`.

## WATT Read Frame

Request:

```text
head version addr func start_hi start_lo count_hi count_lo crc_hi crc_lo tail
```

- Default head: `0x7e`
- Alternate request head from the app: `0x1e`
- Version: `0x00` unless an info payload is included, then `0x01`
- Address: `0x01`
- Read function: `0x03`
- Tail: `0x0d`
- CRC: standard Modbus CRC16 over bytes before CRC, written big-endian.

Confirmed read commands:

- Product info, address `146`: `7e 00 01 03 00 92 00 00 9f 22 0d`
- Analog values, address `140`: `7e 00 01 03 00 8c 00 00 99 42 0d`
- Warnings, address `141`: `7e 00 01 03 00 8d 00 00 59 13 0d`
- Cell characteristics, address `50`: `7e 00 01 03 00 32 00 00 bd 22 0d`
- Collection board, address `30`: `7e 00 01 03 00 1e 00 00 74 e3 0d`
- Protection parameters, address `70`: `7e 00 01 03 00 46 00 00 a7 62 0d`

## Confirmed Live Data

From the laptop BLE probe on May 24, 2026, with battery identity redacted:

- Product: software `1.0`, manufacturer `40`, serial `0001`
- Cell count: `16`
- Pack voltage: about `53.29 V`
- SOC: `90%`
- Current: `0 A`
- Remaining capacity: `87.0 Ah`
- Nominal/reported capacity: `100.0 Ah`
- Cycle count: `1`
- Temperatures: six probes, about `29-30 C`

Run the laptop probe:

```powershell
$py = Join-Path $env:USERPROFILE '.platformio\penv\Scripts\python.exe'
& $py tools\humsienk_ble_probe.py
```
