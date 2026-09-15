# Drive Modes and Cruise Control

[![standard-readme compliant](https://img.shields.io/badge/readme%20style-standard-brightgreen.svg?style=flat-square)](https://github.com/RichardLitt/standard-readme)

> A Cyberpunk 2077 RED4ext plugin that makes keyboard driving calmer and more precise, with Default, Sport and Gentle levels for throttle, brake and steering, and cruise control that really holds your speed, all configurable in-game.

Vanilla keyboard driving is all or nothing: holding W floors the throttle, tapping A or D snaps the wheels, and there is no way to keep a steady speed. Drive Modes and Cruise Control shapes those inputs every frame before the car receives them. Throttle, brake and steering stay at calmer levels, a Sport key gives full power and a Gentle key gives soft curves and careful parking, steering eases in and calms down at speed, and cruise control holds a set speed on flat roads, uphill and downhill. Every option and every key binding lives in the in-game Mod Settings menu and applies instantly.

It runs as a RED4ext plugin (`ImmersiveDriving.dll`) with redscript for the in-game integration, and is inspired by [Immersive Driving by Jo3yization](https://www.nexusmods.com/cyberpunk2077/mods/5293).

> [!NOTE]
> **Made with AI.** This mod was mostly created by Claude, Anthropic's AI assistant: the code, the reverse engineering of the driving inputs, and the documentation. I directed the design and tested it in-game.

## Table of Contents

- [Background](#background)
- [Install](#install)
- [Usage](#usage)
- [How It Works](#how-it-works)
- [Development](#development)
- [Maintainers](#maintainers)
- [Contributing](#contributing)
- [Support](#support)
- [Acknowledgements](#acknowledgements)
- [License](#license)

## Background

The original Immersive Driving remaps keys in the game's input XML, so W gives partial throttle and the mouse thumb buttons give very low "cruise" throttle. That approach is limited by what the input files can express: a key can only produce a fixed value, so the "cruise" buttons slowly gain or lose speed depending on the car and the road, and nothing can be changed without editing files.

This mod works one level deeper. A native plugin sees what the game computed from your keys each tick and replaces it with shaped values right before the vehicle uses them. That makes real feedback control possible, so cruise control measures the speed and adjusts throttle and brakes to hold it.

What it provides:

- **Default, Sport and Gentle levels** - throttle, brake and steering each have three levels: Default (60%, 50% and 75% by default), Sport while holding the Sport key (100%), and Gentle while holding the Gentle key (25%, 25% and 50%).
- **Smooth, speed-sensitive steering** - steering eases in while you hold a direction, so short taps make small corrections, and is gradually reduced at high speed for stable lane changes. Letting go is instant.
- **Cruise control that holds speed** - on, off, speed up and down in steps of 5, and resume, like a real car. Accelerating overtakes and then returns to the cruise speed (or adopts the new one), braking or the handbrake cancels, and it brakes lightly downhill so it does not run away.
- **Everything in-game** - every value, the key bindings, speed units matching the car's dashboard or HUD speedometer, messages and sounds are in Mod Settings and apply immediately.
- **Controller friendly** - the levels apply to keyboard driving by default, since triggers and sticks are already analog. Cruise control works with any input device.

## Install

### Requirements

- Cyberpunk 2077 patch 2.31
- [RED4ext](https://www.nexusmods.com/cyberpunk2077/mods/2380) 1.29.0 or newer
- [redscript](https://www.nexusmods.com/cyberpunk2077/mods/1511) 0.5.31
- [Input Loader](https://www.nexusmods.com/cyberpunk2077/mods/4575), for the mod's key bindings
- [Mod Settings](https://www.nexusmods.com/cyberpunk2077/mods/4885) 0.2.21 or newer, for in-game configuration (needs [ArchiveXL](https://www.nexusmods.com/cyberpunk2077/mods/4198))

Without Mod Settings the mod still works with the default settings and keys.

### Tested with

Tested on the Welcome to Night City (WTNC) Wabbajack mod list, version 2026.1.3, installed with Mod Organizer 2:

| Component | Version |
| --- | --- |
| Cyberpunk 2077 (Steam) | 2.31 (3.0.80.51928) |
| RED4ext | 1.29.1 |
| redscript | 0.5.31 |
| Input Loader | 0.2.3 (the DLL reports 0.1.1) |
| Mod Settings | 0.2.21 |
| ArchiveXL | 1.26.1 |
| Codeware | 1.18.1 |
| TweakXL | 1.11.3 |
| Cyber Engine Tweaks | 1.37.1 |

Codeware, TweakXL and Cyber Engine Tweaks are not required; they are listed because they were part of the tested setup.

### Recommended install

Install the release archive with a mod manager, or extract it into the game folder (the one that contains `bin`, `r6` and `red4ext`). It adds:

```text
red4ext/plugins/ImmersiveDriving/ImmersiveDriving.dll
red4ext/plugins/ImmersiveDriving/scripts/*.reds
r6/input/ImmersiveDriving.xml
```

To uninstall, delete `red4ext/plugins/ImmersiveDriving` and `r6/input/ImmersiveDriving.xml`. The mod does not change saves.

### Compatibility

- **Remove the original Immersive Driving** and any other mod that replaces `r6/config/inputUserMappings.xml` or `inputContexts.xml`. Input Loader needs those files to be vanilla, otherwise no mod's custom keys load.
- Only the player's own driving is affected; traffic, AutoDrive, remote control, quickhacks and scripted scenes are left alone.
- Other plugins that also rewrite the car's driving inputs every frame, such as steering wheel plugins or flight mods while flying, may combine with this mod in unexpected ways. Disable the overlapping features in one of them.

## Usage

Drive as usual and the car uses the calmer Default levels. Hold **Left Shift** for Sport and **Left Alt** for Gentle. Switch cruise control on or off with **Mouse 5**, and change the cruise speed in steps of 5 with **Page Up** and **Page Down**. Everything, including the keys, is configured in **Mods > Drive Modes and Cruise Control** from the main menu or the pause menu.

See the **[Usage and Configuration Guide](docs/README.md)** for the drive modes, cruise control, speed units, and every setting with its default.

### Documentation

- [Usage and Configuration Guide](docs/README.md)
- [FAQ and Troubleshooting](docs/faq.md)
- [Changelog](docs/changelog.md)

## How It Works

Every tick the game zeroes the vehicle's driving inputs and recomputes them from the input actions, updates the camera inputs in `vehicle::BaseObject::UpdateVehicleCameraInput`, and then lets the vehicle consume the result. The plugin hooks that camera update through its RED4ext address hash, so it runs after the driving inputs are known and before anything uses them. For the player's car only, it reads the throttle, brake, handbrake and steering the game produced, shapes them, and writes them back.

- **Throttle, brake and steering shaping** scale the game's value by the level for the active key: Sport, Gentle or Default. Default and Gentle steering are also scaled by speed, and steering eases in at a limited rate.
- **Cruise control** is a PI controller on the measured speed. It follows a target that moves at the configured speed change rate, has anti-windup, limits how fast throttle and brake change, and latches the brakes on when clearly over the target speed. The gains are tuned against a vehicle model covering sports cars, trucks and motorcycles on flat roads and hills (see `tests/CoreTests.cpp`).
- **Game state** comes from redscript. It tracks the driver seat through the player state machine, checks the quest driving restrictions, quickhacks and remote control, forwards the mod's keys, and shows the messages. The plugin also checks AutoDrive every tick.

The driving input offsets are specific to patch 2.31. They were confirmed by disassembling the game's per-tick input update, and the plugin only loads on that patch.

## Development

### Prerequisites

- Visual Studio 2026 or 2022 with the C++ workload (x64)
- CMake 3.25 or newer
- An internet connection for the first configure, which downloads a pinned RED4ext.SDK

### Build and test

```bash
cmake --preset default                         # Visual Studio 2026; use default-vs2022 for 2022
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

The plugin is `build/Release/ImmersiveDriving.dll`. The driving logic in `src/core` has no game dependencies and is covered by `tests/CoreTests.cpp`, including the cruise control simulations. To build only the tests, configure with `-DIMMERSIVE_DRIVING_BUILD_PLUGIN=OFF`.

### Install into the game

```bash
cmake --install build --config Release --prefix "C:/path/to/Cyberpunk 2077"
```

### Check the scripts without launching the game

```powershell
./tools/check-scripts.ps1 -GameDir "C:/path/to/Cyberpunk 2077"
```

This compiles `scripts/` against the game's own script bundle with the redscript compiler, both with and without Mod Settings.

### Package a release

```powershell
./tools/package.ps1
```

This builds, runs the tests and writes `dist/ImmersiveDriving-<version>.zip`, laid out for the game folder.

### Project layout

| Path | Contents |
| --- | --- |
| `src/core` | Game-independent driving logic: settings, input shaping, cruise control |
| `src/plugin` | RED4ext plugin: entry point, vehicle hook, native functions, logging |
| `scripts` | redscript: Mod Settings page, driving state tracking, keys, messages, speed units |
| `input` | Input Loader actions and default key bindings |
| `tests` | Driving logic tests and vehicle model |
| `tools` | Script check, packaging, and a Mod Settings stub for the script check |
| `docs` | Usage guide, FAQ, changelog, and the Nexus page |

## Maintainers

- [@ArthurHub](https://github.com/ArthurHub)

## Contributing

PRs accepted. For larger changes, please open an issue first to discuss what you would like to change.

Code style is enforced by clang-format and pre-commit. After cloning, run `pre-commit install` once so local checks run before commits. See [CLAUDE.md](CLAUDE.md) for repository architecture, build expectations, and coding conventions.

## Support

If you like my work, consider helping out.

[![become a patron](https://theartofdev.wordpress.com/wp-content/uploads/2025/06/become_a_patron_button.png)](https://patreon.com/theartofdev)

## Acknowledgements

- [Jo3yization](https://www.nexusmods.com/cyberpunk2077/mods/5293) for Immersive Driving, the inspiration for this mod.
- [jackhumbert](https://github.com/jackhumbert) for Let There Be Flight, whose vehicle input research and hook this builds on, and for Mod Settings and Input Loader.
- [natpoh](https://github.com/natpoh/cp2077-wheel-mod-moza) for the steering wheel plugin that probed the 2.31 driving input offsets.
- [WopsS](https://github.com/WopsS/RED4ext) for RED4ext, [jac3km4](https://github.com/jac3km4/redscript) for redscript, and [psiberx](https://github.com/psiberx/cp2077-archive-xl) for ArchiveXL.

## License

[GPL-3.0](LICENSE) © 2026 Arthur T
