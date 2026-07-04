# Changelog

Full notes for each release live in [`docs/releases/`](docs/releases/) and on the
[GitHub Releases page](https://github.com/Picklepc/R48Display/releases). Each GitHub
release shows only that version's notes.

| Version | Summary |
|---|---|
| [v0.4.0](docs/releases/v0.4.0.md) | OTA auto-rollback on bad boot; wrong-file upload guard; version picker to install/roll back to any release. |
| [v0.3.7](docs/releases/v0.3.7.md) | Precise pay timekeeping (per-session log); split/back-date correction tool; live current-day hours. |
| [v0.3.6](docs/releases/v0.3.6.md) | One-click web USB installer; device self-updates over Wi-Fi from GitHub releases. |
| [v0.3.5](docs/releases/v0.3.5.md) | Pay Records; Maintenance-page crash fix; web-UI security hardening. |
| [v0.3.4](docs/releases/v0.3.4.md) | Weather display + 7-day forecast; ESP32-S3-Touch-LCD-1.85C support. |
| [v0.3.2](docs/releases/v0.3.2.md) | Animation/screensaver system; 15 themes reworked. |
| [v0.3.1](docs/releases/v0.3.1.md) | 256 KB NVS partition; settings-persistence fixes; touch navigation; AP privacy. |
| [v0.2.3](docs/releases/v0.2.3.md) | Battery power-off control; shutdown + NVS persistence fixes. |
| [v0.2.2](docs/releases/v0.2.2.md) | LVGL dashboard/battery/status refinements; temperature fixes. |
| [v0.2.1](docs/releases/v0.2.1.md) | LVGL layout; °F/°C + clock-format prefs; Ryobi 48V field guide. |
| [v0.2.0](docs/releases/v0.2.0.md) | Configurable display title; redesigned dashboard; settings reorg. |
| [v0.1.0](docs/releases/v0.1.0.md) | Initial release: BMS profiles, MQTT/HA, degradation, maintenance, hour tracking. |

**Upgrade note:** OTA from v0.3.0 or later is safe (unchanged partition table). Upgrading
from 0.2.x or earlier requires a one-time USB flash of the merged binary — see
[v0.3.1 notes](docs/releases/v0.3.1.md).
