# Keychron OpenRGB

OpenRGB plugin that adds stock-firmware QMK/VIA keyboards with RGB Matrix as OpenRGB devices.
It talks to the VIA raw HID interface (usage page `0xFF60`) and needs no reflashing.

- **Direct** mode: the whole keyboard is one LED, driven in real time (works with the Effects plugin, sync, SDK clients). Never written to EEPROM.
- **Hardware effects**: every effect enabled in the firmware, with brightness, speed and color; "Save to device" writes to EEPROM.
- **Custom effects**: the firmware keeps animating per key while the plugin changes the effect, hue, saturation, brightness and speed over time.

Per-key lighting is not possible with stock VIA firmware.

## Custom effects

Listed after the hardware effects in the mode list. They are set up only under **Settings → Keychron OpenRGB** — brightness, colors and speeds included; on the Devices tab their controls are inactive. Changes apply immediately.

| Mode | What it does | Options |
|---|---|---|
| Hue Cycle | Walks the hue of a firmware effect round the color wheel; reactive effects (Solid Splash, Reactive Multiwide) make every keypress a new color | firmware effect, brightness, saturation, effect speed, hue speed |
| Audio Reactive | Bass from the default output device pulses brightness and/or effect speed | firmware effect, brightness, bass/treble colors, what bass drives, fixed or bass→treble color, sensitivity, minimum brightness, effect speed |
| System Monitor | Colors a firmware effect by CPU, RAM or GPU (3D) load | source, firmware effect, brightness, low/high colors and thresholds, smoothing, effect speed |
| Effect Playlist | Plays a queue of firmware effects with a brightness fade between them; each entry can have its own duration and color | queue, brightness, default color, duration, fade, effect speed, shuffle, random color |
| Day / Night | Day look by day, night look by night, with a gradual transition | firmware effect, day/night brightness and colors, start times, transition, effect speed |

The settings tab follows the OpenRGB interface language (English and Russian for now). Mode names stay in English: OpenRGB matches profile modes and "Direct" by name.

Custom effects and Direct are driven by the PC; when OpenRGB closes the keyboard goes back to the firmware effect it showed before. Their effect thread only runs while one of them is selected, and wakes as rarely as the effect allows (60 Hz for audio, 2 Hz for system load, once a minute for day/night).

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
