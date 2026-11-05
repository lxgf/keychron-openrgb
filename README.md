# OpenRGB VIA RGB Matrix plugin

OpenRGB plugin that adds stock-firmware QMK/VIA keyboards with RGB Matrix as OpenRGB devices.
It talks to the VIA raw HID interface (usage page `0xFF60`) and needs no reflashing.

- **Direct** mode: the whole keyboard is one LED, driven in real time (works with the Effects plugin, sync, SDK clients). Never written to EEPROM.
- **Hardware effects**: every effect enabled in the firmware, with brightness, speed and color; "Save to device" writes to EEPROM.
- **Custom effects**: the firmware keeps animating per key while the plugin changes the effect, hue, saturation, brightness and speed over time.

Per-key lighting is not possible with stock VIA firmware.

## Custom effects

Listed after the hardware effects in the mode list. Brightness, speed and colors are the normal per-mode controls on the Devices tab; the other options are under **Settings → VIA RGB Matrix** and apply immediately.

| Mode | What it does | Mode controls | Extra options |
|---|---|---|---|
| Hue Cycle | Walks the hue of a firmware effect round the color wheel; reactive effects (Solid Splash, Reactive Simple) make every keypress a new color | speed = cycle time | firmware effect, saturation, effect speed |
| Audio Reactive | Bass from the default output device pulses brightness and/or effect speed | speed = resting effect speed; colors: bass, treble | firmware effect, what bass drives, fixed or bass→treble color, sensitivity, minimum brightness |
| System Monitor | Colors a firmware effect by CPU, RAM or GPU (3D) load | colors: low, high | source, firmware effect, low/high thresholds, smoothing |
| Effect Playlist | Cycles through firmware effects with a brightness fade between them | speed = effect speed; color | effects, duration, fade, shuffle, random color |
| Day / Night | Day color and brightness by day, night color and dimmed brightness by night, with a gradual transition | brightness = day; colors: day, night | firmware effect, day/night start, transition, night brightness |

The settings tab follows the OpenRGB interface language (English and Russian for now). Mode names stay in English: OpenRGB matches profile modes and "Direct" by name.

Custom effects are driven by the PC and stop when OpenRGB closes; the keyboard then keeps its last state until it is unplugged. Their effect thread only runs while one of them is selected, and wakes as rarely as the effect allows (60 Hz for audio, 2 Hz for system load, once a minute for day/night).

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
