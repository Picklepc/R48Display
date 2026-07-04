# Firmware Binaries

Most people don't need these files directly:

- **New board?** Use the web installer — open **https://picklepc.github.io/R48Display/**
  in Chrome or Edge and click Install. No download required.
- **Already running R48Display?** Update from the device itself under
  **Settings → Firmware Update**.

The merged binary below is for manual or recovery flashing. It is the latest
release for the Waveshare ESP32-S3-Touch-LCD-1.85 (and 1.85C variant).

```text
firmware/R48Display-v0.4.1-merged.bin
```

Flash at offset `0x0` — includes bootloader, partition table, and app:

```sh
esptool.py --chip esp32s3 --baud 460800 write_flash 0x0 R48Display-v0.4.1-merged.bin
```

Or use the [ESP32 Flash Download Tool](https://www.espressif.com/en/support/download/other-tools) — select the merged bin, set offset to `0x0`.

**OTA from v0.3.0 or later is safe.** Full flash required only when upgrading from pre-v0.3.0 (partition layout changed in 0.3.0).

For all releases including older versions, see the [GitHub Releases page](https://github.com/Picklepc/R48Display/releases) or [`CHANGELOG.md`](../CHANGELOG.md).
