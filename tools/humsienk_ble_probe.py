#!/usr/bin/env python3
"""Read-only BLE probe for the HumsiENK / Humsienk WATT BMS profile."""

import argparse
import asyncio
import json
import time

from bleak import BleakClient, BleakScanner

DEFAULT_NAME = ""
DEFAULT_ADDRESS = ""

NOTIFY_UUID = "0000fff1-0000-1000-8000-00805f9b34fb"
WRITE_UUID = "0000fff2-0000-1000-8000-00805f9b34fb"
AUTH_UUID = "0000fffa-0000-1000-8000-00805f9b34fb"

DP_COLLECTION_BOARD = 30
DP_CELL_CHARACTERISTICS = 50
DP_PROTECTION_PARAMETERS = 70
DP_ANALOG_QUANTITY = 140
DP_WARNING_INFO = 141
DP_PRODUCT_INFO = 146


def crc16_modbus(data: bytes) -> int:
    crc = 0xFFFF
    for byte in data:
        crc ^= byte
        for _ in range(8):
            if crc & 1:
                crc = (crc >> 1) ^ 0xA001
            else:
                crc >>= 1
    return crc & 0xFFFF


def watt_read(address: int, *, head: int = 0x7E, read_count: int = 0, info: bytes | None = None) -> bytes:
    frame = bytearray([head, 0x01 if info else 0x00, 0x01, 0x03])
    frame += address.to_bytes(2, "big")
    frame += read_count.to_bytes(2, "big")
    if info:
        frame += info
    frame += crc16_modbus(frame).to_bytes(2, "big")
    frame.append(0x0D)
    return bytes(frame)


def hexs(data: bytes) -> str:
    return data.hex(" ")


def ascii_preview(data: bytes) -> str:
    return "".join(chr(byte) if 32 <= byte < 127 else "." for byte in data)


class WattReassembler:
    def __init__(self) -> None:
        self.buffer = bytearray()

    def feed(self, chunk: bytes) -> list[bytes]:
        out: list[bytes] = []
        self.buffer += chunk
        while self.buffer:
            if self.buffer[0] != 0x7E:
                out.append(bytes([self.buffer.pop(0)]))
                continue
            if len(self.buffer) < 8:
                break
            expected = int.from_bytes(self.buffer[6:8], "big") + 11
            if len(self.buffer) < expected:
                break
            out.append(bytes(self.buffer[:expected]))
            del self.buffer[:expected]
        return out


def parse_product(data: bytes) -> dict[str, str]:
    return {
        "software": data[0:20].decode("ascii", "ignore").strip("\x00 ").strip(),
        "manufacturer": data[20:40].decode("ascii", "ignore").strip("\x00 ").strip(),
        "serial": data[40:60].decode("ascii", "ignore").strip("\x00 ").strip(),
    }


def parse_analog(data: bytes) -> dict[str, object]:
    pos = 0
    cell_count = data[pos]
    pos += 1
    cell_volts = []
    for _ in range(cell_count):
        cell_volts.append(int.from_bytes(data[pos : pos + 2], "big") / 1000.0)
        pos += 2

    temp_count = data[pos]
    pos += 1
    temps_c = []
    for _ in range(temp_count):
        temps_c.append((int.from_bytes(data[pos : pos + 2], "big") - 2730) / 10.0)
        pos += 2

    current_raw = int.from_bytes(data[pos : pos + 2], "big")
    pos += 2
    pack_v = int.from_bytes(data[pos : pos + 2], "big") / 100.0
    pos += 2
    remaining_ah = int.from_bytes(data[pos : pos + 2], "big") / 10.0
    pos += 2
    capacity_ah = int.from_bytes(data[pos : pos + 2], "big") / 10.0
    pos += 2
    cycles = int.from_bytes(data[pos : pos + 2], "big")
    pos += 2
    soc = int.from_bytes(data[pos + 2 : pos + 4], "big") if pos + 4 <= len(data) else None

    return {
        "cell_count": cell_count,
        "cell_volts": cell_volts,
        "temp_count": temp_count,
        "temps_c": temps_c,
        "current_raw": current_raw,
        "pack_v": pack_v,
        "remaining_ah": remaining_ah,
        "capacity_ah": capacity_ah,
        "cycles": cycles,
        "soc": soc,
    }


def parse_frame(frame: bytes) -> dict[str, object]:
    if len(frame) < 11 or frame[0] != 0x7E or frame[-1] != 0x0D:
        return {"raw_hex": hexs(frame), "raw_ascii": ascii_preview(frame)}
    start = int.from_bytes(frame[4:6], "big")
    data_len = int.from_bytes(frame[6:8], "big")
    data = frame[8 : 8 + data_len]
    parsed: dict[str, object] = {
        "version": frame[1],
        "addr": frame[2],
        "func": frame[3],
        "start": start,
        "len": data_len,
        "data_hex": hexs(data),
        "data_ascii": ascii_preview(data),
        "crc": hexs(frame[-3:-1]),
    }
    if start == DP_PRODUCT_INFO and data_len >= 60:
        parsed["product"] = parse_product(data)
    elif start == DP_ANALOG_QUANTITY:
        parsed["analog"] = parse_analog(data)
    return parsed


async def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--name", default=DEFAULT_NAME, help="Advertised BMS name to probe")
    parser.add_argument("--address", default=DEFAULT_ADDRESS, help="BLE address to probe")
    args = parser.parse_args()

    device = None
    if args.address:
        device = await BleakScanner.find_device_by_address(args.address, timeout=12.0)
    if device is None and args.name:
        device = await BleakScanner.find_device_by_name(args.name, timeout=12.0)
    if device is None:
        target = args.address or args.name
        if target:
            raise SystemExit(f"battery not found: {target}")
        raise SystemExit("pass --address <BLE address> or --name <advertised name>")

    reasm = WattReassembler()

    def on_notify(sender, payload: bytearray) -> None:
        chunk = bytes(payload)
        print(f"notify {sender}: {hexs(chunk)}  {ascii_preview(chunk)}")
        for frame in reasm.feed(chunk):
            print(json.dumps(parse_frame(frame), indent=2))

    async with BleakClient(device, timeout=25.0) as client:
        print(json.dumps({"connected": client.is_connected, "mtu": getattr(client, "mtu_size", None)}, indent=2))
        await client.start_notify(NOTIFY_UUID, on_notify)
        await client.write_gatt_char(AUTH_UUID, b"HiLink", response=True)
        print("auth written: HiLink")
        await asyncio.sleep(0.8)

        commands = [
            ("product", watt_read(DP_PRODUCT_INFO)),
            ("analog", watt_read(DP_ANALOG_QUANTITY)),
            ("warnings", watt_read(DP_WARNING_INFO)),
            ("cell_characteristics", watt_read(DP_CELL_CHARACTERISTICS)),
            ("collection_board", watt_read(DP_COLLECTION_BOARD)),
            ("protection", watt_read(DP_PROTECTION_PARAMETERS)),
        ]
        for name, command in commands:
            print(f"write {name}: {hexs(command)}")
            await client.write_gatt_char(WRITE_UUID, command, response=True)
            await asyncio.sleep(2.5)

        await asyncio.sleep(1)


if __name__ == "__main__":
    asyncio.run(main())
