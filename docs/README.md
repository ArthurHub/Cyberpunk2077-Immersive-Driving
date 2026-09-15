# Usage and Configuration Guide

Keyboard driving in Cyberpunk 2077 is all or nothing: holding W floors the throttle, tapping A or D snaps the wheels, and nothing keeps a steady speed. Drive Modes and Cruise Control shapes those inputs every frame before the car receives them. Throttle, brake and steering stay at calmer levels, a Sport key gives you everything when you need it, a Gentle key gives soft curves and careful parking, and cruise control holds the speed you set on flat roads, uphill and downhill. Everything is configured in-game and applied as soon as you accept the change.

Cruise down the highway at a steady 60 with the radio on, drift through a lane change with a light touch of the steering, and hold Sport the moment a chase starts.

For installation, requirements, and credits see the [main README](../README.md). For troubleshooting and common questions see the [FAQ](faq.md). For what changed in each release see the [Changelog](changelog.md).

## Contents

- [Drive Modes](#drive-modes)
- [Default Key Bindings](#default-key-bindings)
- [Steering](#steering)
- [Cruise Control](#cruise-control)
- [Speed Units](#speed-units)
- [Keyboard, Controller and Motorcycles](#keyboard-controller-and-motorcycles)
- [In-Game Configuration](#in-game-configuration)
- [Settings Reference](#settings-reference)
- [Logs and Debug Logging](#logs-and-debug-logging)

## Drive Modes

Throttle, brake and steering each have three levels, as a percentage of what the key would give in the vanilla game:

| Mode | When | Throttle | Brake | Steering |
| --- | --- | --- | --- | --- |
| **Default** | No mode key held | 60% | 50% | 75% |
| **Sport** | Holding the Sport key | 100% | 100% | 100% |
| **Gentle** | Holding the Gentle key | 25% | 25% | 50% |

- **Default** is how the car drives most of the time: a calmer throttle, softer braking, and less twitchy steering.
- **Sport** is the vanilla car: full throttle for overtaking and chases, full braking, and full steering lock. Speed-sensitive steering does not reduce Sport steering.
- **Gentle** is for parking, tight streets, slow traffic, and soft lane changes.
- If both keys are held, **Sport wins**.
- The levels apply at once, with no build-up or release delay. Brake also covers reverse throttle.

All nine values are adjustable from 0 to 100%.

## Default Key Bindings

Rebind the keys in **Mods > Drive Modes and Cruise Control > Key Bindings**, from the main menu or the pause menu.

| Action | Default key | What it does |
| --- | --- | --- |
| Sport Mode | Left Shift | Hold for the Sport throttle, brake and steering levels |
| Gentle Mode | Left Alt | Hold for the Gentle throttle, brake and steering levels |
| Cruise control on / off | Mouse 5 (front thumb button) | Switches cruise control on at the current speed, or off |
| Cruise speed up | Page Up | Raises the cruise speed by 5. While cruise control is off, switches it back on at the last cruise speed |
| Cruise speed down | Page Down | Lowers the cruise speed by 5. While cruise control is off, switches it on at the current speed |

The keys only do something while you drive. The mouse wheel works well for cruise speed up and down, but it also zooms the vehicle camera in the vanilla game.

## Steering

- **Smooth steering** (on by default) builds steering up while you hold left or right, reaching the level of the current mode in the **steer-in time** (0.3 s). Short taps make small corrections. Letting go, easing off, and changing direction are always instant.
- **Speed-sensitive steering** (on by default) gradually reduces Default and Gentle steering at speed, for stable lane changes and long highway curves. Below 40 km/h steering is never reduced, and at 140 km/h and above it is 65% of the mode's level. Sport steering is never reduced.

All speeds in these settings are true speeds in km/h (40 km/h is about 25 mph, 140 km/h about 87 mph).

## Cruise Control

- **Switch it on** with the cruise key above the minimum speed (20 km/h by default). The cruise speed is your current speed rounded to a step of 5, for example 57 becomes 55.
- **Change the speed** with cruise speed up and down, always in steps of 5 (50, 55, 60). The car speeds up or slows down smoothly at the speed change rate.
- **Accelerating** always works. When you let go, cruise control returns to the cruise speed, like a real car. With *After using the pedals* set to *Use new speed*, the speed you let go at (rounded to 5) becomes the new cruise speed.
- **Braking** switches cruise control off. With *Brake cancels cruise control* off, braking only pauses it until you let go of the brake.
- It also switches off for the **handbrake**, a **sudden impact**, staying **far below the minimum speed** (stuck in traffic), **leaving the driver seat**, **scenes**, **AutoDrive**, and vehicle **quickhacks** or **remote control**.
- It **brakes lightly** downhill or after lowering the cruise speed so the car does not run away, up to the maximum cruise braking. Turn *Brake to hold speed* off to only coast.
- Steering, the Sport and Gentle keys, and the horn keep working while cruising.
- A short on-screen message and a click confirm each change. Both can be turned off.

Cruise control works with keyboard and controller alike.

## Speed Units

The game's speedometers show a number that runs well above the true speed, and cruise control uses the number you actually see. Pick the one that matches the display you read in **General > Speed units**:

- **Car dashboard** (default) — the display inside the car. It shows the same number whatever the game's metric or imperial setting.
- **HUD speedometer** — the third person speedometer, which follows the game's *Speedometer units* setting.
- **True km/h** and **True mph** — the real speed, which is lower than either speedometer.

## Keyboard, Controller and Motorcycles

- **Keyboard only by default.** Triggers and sticks are already analog, so throttle, brake and steering levels only apply to keyboard driving. Turn on *Shape controller input too* to apply them to a controller as well. Cruise control works with any device.
- **Controller buttons** can't be rebound in Mod Settings. To use one, add a `<button id="IK_Pad_..."/>` line to the matching mapping in `r6/input/ImmersiveDriving.xml`.
- **Motorcycles** are supported, and can be switched off separately from cars.
- **Lean keys.** In the vanilla game Left Shift and Left Ctrl lean forward and back (motorcycles, and cars in the air), and the game scales steering down while a lean key is held. While the Sport or Gentle key is held on the keyboard, the mod restores full steering and drops that lean, so Sport on Left Shift steers fully and does not lean.

## In-Game Configuration

Open **Mods > Drive Modes and Cruise Control** from the main menu or the pause menu (this needs Mod Settings). Changes apply as soon as you accept them, including while driving, with no reload. Without Mod Settings the mod runs with the default values and keys.

## Settings Reference

### General

| Setting | Default | Meaning |
| --- | --- | --- |
| Enable Drive Modes and Cruise Control | On | Master switch. When off, driving is completely vanilla. |
| Speed units | Car dashboard | Units for cruise control speeds, see [Speed Units](#speed-units). |
| Show messages | On | Short on-screen messages when cruise control changes. |
| Play sounds | On | A soft click when cruise control switches on or off. |
| Cars | On | Use the mod in cars, vans and trucks. |
| Motorcycles | On | Use the mod on motorcycles. |
| Shape controller input too | Off | Apply the throttle, brake and steering levels to a controller. |

### Throttle, Brake and Steering

| Setting | Default |
| --- | --- |
| Default throttle | 60% |
| Sport throttle | 100% |
| Gentle throttle | 25% |
| Default brake | 50% |
| Sport brake | 100% |
| Gentle brake | 25% |
| Default steering | 75% |
| Sport steering | 100% |
| Gentle steering | 50% |

### Advanced Steering

| Setting | Default | Meaning |
| --- | --- | --- |
| Smooth steering | On | Steering builds up while holding a direction. |
| Steer-in time | 0.3 s | Time to reach the steering level. |
| Speed-sensitive steering | On | Less Default and Gentle steering at high speed. |
| Full steering below | 40 km/h | Below this true speed steering is never reduced. |
| High speed | 140 km/h | True speed at which steering reaches its high speed level. |
| Steering at high speed | 65% | Steering at and above the high speed, as a percentage of the mode's level. |

### Cruise Control

| Setting | Default | Meaning |
| --- | --- | --- |
| Enable cruise control | On | |
| Minimum speed | 20 km/h | Cruise control only engages above this true speed, and switches off when the car stays far below it. |
| Speed change rate | 5 km/h per second | How quickly the car reaches a new cruise speed. |
| Responsiveness | 100% | How firmly the speed is held. Lower feels softer, higher reacts faster to hills. |
| After using the pedals | Resume set speed | *Resume set speed* returns to the cruise speed after accelerating. *Use new speed* keeps the speed you let go at. |
| Brake cancels cruise control | On | When off, braking only pauses cruise control. |
| Handbrake cancels cruise control | On | |
| Crashes cancel cruise control | On | Switch off after a sudden impact. |
| Brake to hold speed | On | Brake lightly downhill or after lowering the cruise speed. When off, the car only coasts. |
| Maximum cruise braking | 30% | Most brake cruise control uses. |
| Maximum cruise throttle | 100% | Lower values climb hills and reach a higher cruise speed more gently. |

### Advanced

| Setting | Default | Meaning |
| --- | --- | --- |
| Debug logging | Off | Writes driving values, key presses, and cruise speed changes to the log. |

## Logs and Debug Logging

The mod writes its log to `red4ext/logs/ImmersiveDriving-*.log` in the game folder. With Mod Organizer 2 the logs land in the MO2 `overwrite` folder instead, for example `overwrite/red4ext/logs/`.

A healthy start logs `Hooked vehicle::BaseObject::UpdateVehicleCameraInput` and `Registered native functions`. With **Advanced > Debug logging** on, the log also shows once per second the speed, the inputs the game computed and what the mod wrote, plus every press of the mod's keys and every cruise speed change. That log is the most useful thing to attach to a bug report.
