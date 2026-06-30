# Security Policy

## Supported versions

Only the latest release on `main` receives security fixes.

## Reporting a vulnerability

Please report security issues privately to **patrickchaffee@gmail.com** with the subject line `[R48Display Security]`.

Include:
- A description of the vulnerability and its impact
- Steps to reproduce or a proof-of-concept
- The firmware version and hardware configuration you tested on

You can expect an acknowledgement within 72 hours and a fix or mitigation plan within 14 days for confirmed issues.

Please do not open public GitHub issues for security vulnerabilities.

## Known limitations

### Web interface
- The web UI has no authentication by default. It is intended for use on a trusted LAN only.
- The OTA endpoint (`/update`) is protected by a password stored in NVS. Use a strong OTA password if the device is exposed beyond a trusted network.
- All HTTP traffic is plaintext. Do not expose the device directly to the internet.

### MQTT (when enabled)
- MQTT connections use plaintext port 1883 by default.
- For external broker access, place the device behind a VPN or use a local broker.
- TLS support is planned but not yet implemented.

### Credentials in NVS
- WiFi, AP, OTA, and MQTT passwords are stored in ESP32 NVS flash. Physical access to the device allows extraction via JTAG or flash dump.
- Firmware 0.3.0 moves the default NVS partition to `0xa10000` and expands it to 256 KB. Upgrades from older partition tables require a full USB/fresh flash; OTA alone does not move stored credentials.
- `/api/settings` GET never returns plaintext passwords; it returns boolean `_set` flags only.

### BLE
- BLE advertising is passive (device does not advertise). It scans for and connects to a configured BMS.
- No BLE pairing or authentication is enforced beyond what the BMS itself requires.
