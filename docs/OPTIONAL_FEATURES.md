# Optional Feature Development

The stable release intentionally does not include SD card, GPS, navigation, IMU,
file manager, garage log, map code, charger control, inverter control, or relay
automation.

R48Display should stay a monitor first. Optional features can improve a specific
install category, but they must not make mowers, golf carts, trailers/RVs, power
stations, marine packs, utility carts, or off-grid systems depend on hardware
they do not have.

The release does include a small optional audio-assist module for 1.85-inch board
variants that have the onboard I2S microphone. It is category-gated for mower
work detection, can be disabled from settings, and should not grow into a
general audio feature inside the stable branch.

Starter templates live here:

- `extras/features/sd_card/SdCardFeature.h`
- `extras/features/sd_card/SdCardFeature.cpp`
- `extras/features/navigation/NavigationFeature.h`
- `extras/features/navigation/NavigationFeature.cpp`

These are not compiled into the firmware. They are starting points for separate
branches or forks. Keep each optional feature self-contained and avoid adding
new dependencies to `platformio.ini` until the feature is promoted.

## Promotion Checklist

- The feature builds with the stable project.
- The feature can be disabled without leaving dead routes or UI controls.
- The main BMS dashboard, cell monitor, clock, setup AP, and OTA updater still
  work when the feature is removed.
- The feature does not assume a mower, cart, trailer, power station, or marine
  pack unless it is explicitly scoped to that category.
- No long blocking work runs in `loop()`.
- Any new hardware pins are documented in `include/BoardConfig.h`.
