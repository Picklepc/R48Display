# Firmware Binaries

Release `.bin` files can be placed here for users who want a ready-to-flash
large-pack battery monitor for mowers, golf carts, trailers/RVs, power stations,
marine packs, utility carts, or custom lithium systems.

Generated binaries:

```text
firmware/R48Display-v0.3.0.bin
firmware/R48Display-v0.3.0-merged.bin
```

Use `R48Display-v0.3.0.bin` for OTA or PlatformIO app uploads only after the
0.3.0 partition table is already installed. Use
`R48Display-v0.3.0-merged.bin` with flashing tools that write one complete image
at offset `0x0`, especially after erasing the board or upgrading from 0.2.x.

The 0.3.0 release moves and enlarges NVS to 256 KB. Install it with a full
USB/fresh flash; OTA alone cannot rewrite the partition table. Erase first for a
clean NVS region:

```powershell
esptool.py --chip esp32s3 --port YOUR_PORT erase_flash
esptool.py --chip esp32s3 --port YOUR_PORT write_flash 0x0 R48Display-v0.3.0-merged.bin
```

Build it with:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run -e waveshare_esp32_s3_touch_lcd_1_85
```

Or rebuild and package both public binaries plus SHA-256 checksums:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\package_firmware.ps1
```
