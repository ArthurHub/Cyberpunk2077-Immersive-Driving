# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Drive Modes and Cruise Control is a Cyberpunk 2077 (patch 2.31) mod: a RED4ext C++ plugin plus redscript. The user-visible name (Mod Settings menu, docs, Nexus page, log messages, version resource) is "Drive Modes and Cruise Control"; everything internal keeps the original working name `ImmersiveDriving` (DLL, plugin folder, input XML, script class prefix, natives, Mod Settings class and saved settings section), so renaming internals would break installs and saved settings. The GitHub repo is `ArthurHub/Cyberpunk2077-Immersive-Driving` (linked from `docs/nexus.md`). It shapes the player's keyboard driving inputs every tick (Default, Sport and Gentle levels for throttle, brake and steering, smooth and speed-sensitive steering) and adds cruise control that holds a set speed. Everything is configured in-game through Mod Settings, and the mod's keys come from an Input Loader XML. Inspired by Jo3yization's Immersive Driving (Nexus 5293), which only remaps input XML values.

## Build System

**Prerequisites:** Visual Studio 2026 or 2022 (MSVC, x64), CMake 3.25+. The first configure downloads RED4ext.SDK (pinned commit, SHA256-checked) through FetchContent.

```bash
cmake --preset default                                   # VS 2026; default-vs2022 for VS 2022
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure    # driving logic tests
cmake --install build --config Release --prefix "<game dir>"   # install into the game
```

- `-DIMMERSIVE_DRIVING_BUILD_PLUGIN=OFF` builds only the core library and tests (no SDK download).
- `tools/check-scripts.ps1 -GameDir <game dir>` compiles `scripts/` against the game's `r6/cache/final.redscripts` (or `.bk` when redscript is installed) with redscript-cli 0.5.31, once with the Mod Settings stub in `tools/stubs` and once without. The CLI exits 0 even on failure, so the script checks its output.
- `tools/package.ps1` builds, runs tests, installs to `dist/staging` and zips `dist/ImmersiveDriving-<version>.zip`.
- The CRT is linked statically, so the DLL only depends on KERNEL32 and USER32.

**No in-game automated tests.** `tests/CoreTests.cpp` covers the game-independent logic, including cruise control against a point-mass vehicle model. Anything touching the hook, natives or scripts needs a manual in-game check.

## Code Style

clang-format enforces the C++ style (`.clang-format`, same as the author's F4VR repos): LLVM-based, 180 columns, 4-space indent, namespace indentation, braces on new lines for classes/functions/namespaces. After editing C++ run `pre-commit run --files <changed files>` and let it format.

- **Naming:** camelCase functions and variables, `_camelCase` private members, PascalCase types, UPPER_SNAKE_CASE constants.
- **Namespace:** `immersive_driving` (sub-namespaces `logger`, `natives`, `vehicle_hook`, `vehicle_functions`).
- **Doc comments:** `/** */` always multi-line.
- **redscript:** PascalCase methods like the game scripts, 2-space indent, `ImmersiveDriving` class prefix. No `module` in `scripts/` (Mod Settings reads class names, natives are global).

## Architecture

```text
Input Loader XML ──actions──▶ ImmersiveDrivingSystem (redscript) ──natives──▶ DrivingRuntime (C++) ──▶ DriveController
Mod Settings ──settings──────▶ ImmersiveDrivingSettings.Push()   ──natives──▶ Config
game per-tick input update ─────────────────────────────▶ vehicle hook ──▶ DrivingRuntime::afterInputUpdate
```

| Path | Role |
| --- | --- |
| `src/core/Config.*` | Every native tunable; `setBool/setFloat/setInt` by field name, clamped. Names match `scripts/Settings.reds`. |
| `src/core/DriveController.*` | Pure logic: Default/Sport/Gentle levels (`*NormalPct` fields are the Default level) for throttle, brake and steering (Sport wins), steering ease-in, speed-sensitive steering, lean pairing fix, cruise PI controller, cancel rules, event queue. |
| `src/core/MathUtil.h` | Small math helpers and unit conversions. |
| `src/plugin/Main.cpp` | RED4ext `Main`/`Query`/`Supports`. Registers natives, adds `scripts` to redscript compilation (relative to the DLL), attaches the hook. Runtime is 2.31 only. |
| `src/plugin/VehicleHook.*` | Detour on `vehicle::BaseObject::UpdateVehicleCameraInput` (hash `501486464`). |
| `src/plugin/DrivingRuntime.*` | Singleton shared by the hook and natives, guarded by a mutex, with lock-free player vehicle pointer checks. Reads/writes the input fields. |
| `src/plugin/VehicleFunctions.*` | RTTI calls to `VehicleObject.GetCurrentSpeed` and `IsAutoDriveModeEnabled`. |
| `src/plugin/Natives.*` | Global native functions for redscript; `scripts/Natives.reds` declares the same list. |
| `scripts/Settings.reds` | Mod Settings class (all options and key bindings) and `Push()` to the plugin. |
| `scripts/System.reds` | `ImmersiveDrivingSystem` (ScriptableSystem): player state machine listener, driver/vehicle tracking, driving restrictions, key handling, cruise commands, 10 Hz event poll, on-screen messages. |
| `scripts/Speed.reds` | m/s to car dashboard/HUD speedometer/true km/h/true mph, bisection inverse for the speedometer curve, cruise speeds snapped to steps of 5. |
| `scripts/ModSettingsCompat.reds` | `@if(ModuleExists("ModSettingsModule"))` wrappers so the mod works without Mod Settings. |
| `docs/` | `README.md` usage and configuration guide (every setting and default), `faq.md`, `changelog.md`, `nexus.md` (Nexus short description and BBCode page). Same layout as the author's F4VR mods; update them with user-visible changes. |
| `input/ImmersiveDriving.xml` | Five button actions appended to `VehicleDriveBase`, `VehicleDrive_QuickHackPanel`, `BaseVehicleDriverCombat`. |

### Keeping names in sync

- Config field names: `src/core/Config.cpp` tables and `ImmersiveDrivingSettings.Push()`.
- Natives: `src/plugin/Natives.cpp` registration and `scripts/Natives.reds`.
- Key bindings: the `EInputKey` field names in `Settings.reds` are the `overridableUI` names in the XML (Mod Settings rebinds by that name). Action names in the XML match `OnInputAction` in `System.reds`.
- `CruiseEvent`/`EngageResult` integer values are switched on in `System.reds`.

### Per-tick flow and why it is safe

Game per-tick vehicle input update (patch 2.31, RVAs from `Cyberpunk2077.exe` 3.0.80.51928):

```text
0x2E5368  per-tick vehicle input update
  0x2E53C4  pre-step
  0x2E54FC  driving inputs: zero the block 0x264..0x2A2, then write each value from its action
  0x2E6010  UpdateVehicleCameraInput (hooked): camera fields 0x284..0x290 and flags 0x29B..0x2A1 only
  0x2E54C8  reads flag 0x2A2 only
  vtable[0x380]  the vehicle consumes the inputs
```

Driving input fields on `vehicle::BaseObject` (2.31): `0x264` Accelerate (0..1), `0x268` Decelerate (0..1), `0x26C` Handbrake (0..1), `0x270` accelerate minus decelerate, `0x274` second axis (tank/special mode), `0x278` TurnX steer (-1..1, positive right), `0x27C` LeanFB, `0x280` RockFB. Action CNames are FNV-1a 64 hashes (`Accelerate` = `0xc9e8ff04669cae10`, `TurnX` = `0x3ed85e372e5e12e6`). Because the game rewrites the block from scratch each tick, writing after the hook needs no restore, and the game never sees our values as history. The plugin keeps `0x270` consistent by applying the same pedal deltas, and writes `0x27C` for the lean pairing fix below. Let There Be Flight's SDK fork labels these 4 bytes later (older patch).

The driving input function also forces values at the end (a vtable check that zeroes pedals and applies the handbrake, and an accelerate-quickhack path that forces full throttle). The scripts treat quickhacks, remote control and the `NoDriving`/`VehicleOnlyForward` player status effect tags as "driving not allowed", and the plugin checks AutoDrive every tick, so shaping and cruise control stay out of those states.

### Updating for a new game patch

1. Check the hash is still in `bin/x64/cyberpunk2077_addresses.json` (`501486464`).
2. Disassemble the function at that address and its caller (capstone and pefile in a scratch venv work well; section `.text` starts at RVA `0x1000`, the JSON offsets are section-relative). Confirm the caller still runs driving input, then camera input, then the consumer.
3. Find the driving input function by searching `.text` for `movabs rdx, <FNV-1a of "Accelerate">` and re-read the field offsets it writes. Update the constants in `DrivingRuntime.cpp`.
4. Bump `RED4EXT_V1_RUNTIME_VERSION_*` in `Main.cpp` (and the SDK pin in `CMakeLists.txt` if the SDK needs updating).
5. Re-run `tools/check-scripts.ps1` against the new script bundle, and check the vanilla scripts used by `System.reds` and `Speed.reds` (player state machine enum, `VehicleComponent.GetVehicle/IsDriver`, `SpeedometerUnits`, the `vehicle_ui` curve).

## Known Tooling Quirks

- RED4ext 1.29.x only loads plugins that report RED4ext.SDK 0.5.0 (log: "uses RED4ext.SDK v1.0.0 which is not supported"). `Query` reports `RED4EXT_V1_SDK_VERSION_1_0_0_COMPAT_0_5_0` and `Supports` returns `RED4EXT_API_VERSION_1_COMPAT_0`, which RED4ext 1.30 accepts too.
- Mod Settings `RegisterListenerToClass` only copies accepted values into the object's fields. `OnModSettingsChange()` is only called on objects registered with `RegisterListenerToModifications`, after the fields are updated. The settings object is registered with both, otherwise changes only reach the plugin after a save reload.
- Mouse wheel keys (`IK_MouseWheelUp`/`Down`) as defaults in `input/ImmersiveDriving.xml` crash the game at startup (patch 2.31, Input Loader 0.1.1; found by bisecting launches). The same keys bound through Mod Settings work; both defaults are Page Up/Down. Test any new XML default key with a real launch.
- Under MO2, files the game writes (logs, redscript cache) land in the MO2 instance's `overwrite` folder, for example `overwrite/red4ext/logs/`.

- redscript-cli 1.0 previews report `LESS_VISIBLE_OVERRIDE` for any non-public override of a bundle method (it errors on Mod Settings' own scripts too); `OnAttach`/`OnDetach` are `public` so ours compile with both 0.5.31 and 1.0.
- `static` on global native declarations is a warning in redscript 1.0; `Natives.reds` omits it.
- Vanilla binds Left Shift/Left Ctrl to `LeanFB`, and `pairedAxes LeftStick_Vehicle` pairs it with `TurnX`, so A/D plus a lean key arrives as 0.71/0.71. The Sport and Gentle keys can be those keys, so with the keyboard `DriveController::tick` divides steering by the larger component and zeroes lean while either mod key is held.
- The car's own display (`speedometerLogicController`) shows `GetCurrentSpeed() * vehicle_ui curve` regardless of the units setting. Only the HUD speedometer (`hudCarController`) multiplies by 1.61 for metric. Both are well above the true speed.
- `SimpleMessageType.Vehicle` warning messages are hidden by the HUD and every warning message plays a jingle, so `Notify` uses `UI_Notifications.OnscreenMessage`.
