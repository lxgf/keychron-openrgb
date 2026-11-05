# OpenRGB VIA RGB Matrix plugin

OpenRGB plugin that adds stock-firmware QMK/VIA keyboards with RGB Matrix as OpenRGB devices.
It talks to the VIA raw HID interface (usage page `0xFF60`) and needs no reflashing.

- **Direct** mode: the whole keyboard is one LED, driven in real time (works with the Effects plugin, sync, SDK clients). Never written to EEPROM.
- **Hardware effects**: every effect enabled in the firmware, with brightness, speed and color; "Save to device" writes to EEPROM.

Per-key lighting is not possible with stock VIA firmware.

## Compatibility

| OpenRGB | Plugin API | Qt | Status |
|---|---|---|---|
| 1.1 (Windows x64) | 5 | 6.8.3 | in development |

Plugins load only into the OpenRGB version they were built for.

## Supported keyboards

| Keyboard | VID:PID |
|---|---|
| Keychron C2 Pro (V1) RGB | `3434:0520` |

Other keyboards are added to the allowlist in `src/device/ViaRGBMatrixDevices.cpp`.
Close Keychron Launcher / VIA while using OpenRGB.

## Known issues

- OpenRGB 1.1 logs `Device thread still active in base class destructor` when a plugin device is removed (unplug, plugin unload). It comes from OpenRGB's `RGBController_Virtual` not calling `Shutdown()` and is harmless.

## Build (Windows)

Requirements: Visual Studio with "Desktop development with C++" (MSVC + Windows SDK), Qt 6.8.3 `msvc2022_64`, CMake, Ninja.

```
git clone --recursive <repo>
scripts\build.cmd
```

Copy `build\OpenRGBViaRGBMatrixPlugin.dll` to `%APPDATA%\OpenRGB\plugins\`.

## Protocol check

`tools/via_probe.py` (needs `pip install hidapi`) reads the lighting state and probes the effect count without saving anything.

## License

GPL-2.0-or-later.
