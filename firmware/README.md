# Firmware Binaries

Release `.bin` files can be placed here for users who want a ready-to-flash
large-pack battery monitor for mowers, golf carts, trailers/RVs, power stations,
marine packs, utility carts, or custom lithium systems.

Generated binaries:

```text
firmware/R48Display-v0.1.0.bin
firmware/R48Display-v0.1.0-merged.bin
```

Use `R48Display-v0.1.0.bin` for OTA or PlatformIO app uploads. Use
`R48Display-v0.1.0-merged.bin` with flashing tools that write one complete image
at offset `0x0`.

Build it with:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run -e waveshare_esp32_s3_touch_lcd_1_85
```

Or rebuild and package both public binaries plus SHA-256 checksums:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\package_firmware.ps1
```
