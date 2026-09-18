# Usage and Configuration Guide

Keyboard driving in Cyberpunk 2077 is all or nothing: holding W floors the throttle, tapping A or D snaps the wheels, and nothing keeps a steady speed. Drive Modes and Cruise Control shapes those inputs every frame before the car receives them. Throttle, brake and steering stay at calmer levels, a Sport key gives you everything when you need it, a Gentle key gives soft curves and careful parking, cruise control holds the speed you set on flat roads, uphill and downhill, and a speed limiter keeps you under a limit you set. Everything is configured in-game and applied as soon as you accept the change.

Cruise down the highway at a steady 60 with the radio on, drift through a lane change with a light touch of the steering, and hold Sport the moment a chase starts.

For installation, requirements, and credits see the [main README](../README.md). For troubleshooting and common questions see the [FAQ](faq.md). For what changed in each release see the [Changelog](changelog.md).

## Contents

- [Drive Modes](#drive-modes)
- [Mode Keys: Hold, Toggle, or Tap or Hold](#mode-keys-hold-toggle-or-tap-or-hold)
- [Default Key Bindings](#default-key-bindings)
- [Steering](#steering)
- [Cruise Control](#cruise-control)
- [Speed Limiter](#speed-limiter)
- [On-Screen Indicators](#on-screen-indicators)
- [Speed Units](#speed-units)
- [Keyboard, Controller and Motorcycles](#keyboard-controller-and-motorcycles)
- [In-Game Configuration](#in-game-configuration)
- [Settings Reference](#settings-reference)
- [Logs and Debug Logging](#logs-and-debug-logging)

## Drive Modes

Throttle, brake and steering each have three levels, as a percentage of what the key would give in the vanilla game:

| Mode | When | Throttle | Brake | Steering |
| --- | --- | --- | --- | --- |
| **Default** | No mode active | 60% | 50% | 75% |
| **Sport** | Holding the Sport key, or switched on with it | 100% | 100% | 100% |
| **Gentle** | Holding the Gentle key, or switched on with it | 25% | 25% | 50% |

- **Default** is how the car drives most of the time: a calmer throttle, softer braking, and less twitchy steering.
- **Sport** is the vanilla car: full throttle for overtaking and chases, full braking, and full steering lock. Speed-sensitive steering does not reduce Sport steering.
- **Gentle** is for parking, tight streets, slow traffic, and soft lane changes.
- The mode keys can be held, toggled, or both, see [Mode Keys](#mode-keys-hold-toggle-or-tap-or-hold).
- The levels apply at once, with no build-up or release delay. Brake also covers reverse throttle.

All nine values are adjustable from 0 to 100%.

## Mode Keys: Hold, Toggle, or Tap or Hold

The Sport and Gentle keys each have their own setting in **Key Bindings**: *Sport Mode key* and *Gentle Mode key*.

- **Hold**: the mode lasts while you hold the key.
- **Toggle**: press to switch the mode on, press again to switch it off.
- **Tap or hold** (default): a short tap (under 0.3 s) switches the mode on or off. Holding the key flips the mode only while you hold it, and leaves it as it was when you let go.

What a key does depends on its setting and on which mode is on at the moment. "This mode" is the key's own mode, "the other mode" is the other key's mode.

| Key setting | Right now | Hold the key | Tap the key |
| --- | --- | --- | --- |
| Hold | No mode on | This mode while held, then Default | This mode while pressed |
| Hold | Other mode toggled on | This mode while held, then the other mode again | This mode while pressed |
| Toggle | No mode on | Switches this mode on | Switches this mode on |
| Toggle | This mode on | Switches this mode off | Switches this mode off |
| Toggle | Other mode toggled on | Switches this mode on and the other mode off | Switches this mode on and the other mode off |
| Tap or hold | No mode on | This mode while held, then Default | Switches this mode on |
| Tap or hold | This mode on | Default while held, then this mode again | Switches this mode off |
| Tap or hold | Other mode toggled on | This mode while held, then the other mode again | Switches this mode on and the other mode off |

- Only one mode is on at a time, except while both keys are held, where **Sport wins**.
- A short message and a click confirm each toggle. Both can be turned off in **General**.
- A toggled mode switches off when you leave the driver seat, and when its key is changed to *Hold*.

For example, with Sport on *Tap or hold*: tap Left Shift to drive in Sport, hold Left Shift for a moment of calmer Default driving through a tight corner, and tap it again to go back to Default.

## Default Key Bindings

Rebind the keys in **Mods > Drive Modes and Cruise Control > Key Bindings**, from the main menu or the pause menu.

| Action | Default key | What it does |
| --- | --- | --- |
| Sport Mode | Left Shift | Tap to switch the Sport throttle, brake and steering levels on and off, or hold for them while held |
| Gentle Mode | Left Alt | Tap to switch the Gentle throttle, brake and steering levels on and off, or hold for them while held |
| First key | Mouse 5 (front thumb button) | Switches cruise control on at the current speed, or off. Switches the speed limiter off |
| Second key | Mouse 4 (back thumb button) | Switches the speed limiter on at the current speed rounded up, or off. Switches cruise control off |
| Set speed up | Page Up | Raises the cruise speed or the speed limit by 5, whichever is on. With both off, switches cruise control back on at the last cruise speed |
| Set speed down | Page Down | Lowers the cruise speed or the speed limit by 5, whichever is on. With both off, switches cruise control on at the current speed |

What the first and second keys switch is set by *First key does* and *Second key does*: cruise control, the speed limiter, one of them on a short press and the other on a long press of about half a second, or nothing. Putting both on one key leaves the other key free for something else. A key that uses a long press acts when you let go, cruise control then starts from the speed you had when you pressed it, and a short press switches off what the long press put on.

Whether the Sport and Gentle keys are held, toggled, or both is set by *Sport Mode key* and *Gentle Mode key* (Tap or hold by default), see [Mode Keys](#mode-keys-hold-toggle-or-tap-or-hold). With cruise control and the speed limiter both off, *Set speed keys* can make set speed up and down start the speed limiter instead, or do nothing. The keys only do something while you drive. The mouse wheel works well for set speed up and down, but it also zooms the vehicle camera in the vanilla game.

## Steering

- **Smooth steering** (on by default) builds steering up while you hold left or right, reaching the level of the current mode in the **steer-in time** (0.3 s). Short taps make small corrections. Letting go, easing off, and changing direction are always instant.
- **Speed-sensitive steering** (on by default) gradually reduces Default and Gentle steering at speed, for stable lane changes and long highway curves. Below 40 km/h steering is never reduced, and at 140 km/h and above it is 65% of the mode's level. Sport steering is never reduced.

All speeds in these settings are true speeds in km/h (40 km/h is about 25 mph, 140 km/h about 87 mph).

## Cruise Control

- **Switch it on** with the cruise key above the minimum speed (20 km/h by default). The cruise speed is your current speed rounded to a step of 5, for example 57 becomes 55.
- **Change the speed** with set speed up and down, always in steps of 5 (50, 55, 60). The car speeds up or slows down smoothly at the speed change rate.
- **Accelerating** always works. When you let go, cruise control returns to the cruise speed, like a real car. With *After using the pedals* set to *Use new speed*, the speed you let go at (rounded to 5) becomes the new cruise speed.
- **Braking** switches cruise control off. With *Brake cancels cruise control* off, braking only pauses it until you let go of the brake.
- It also switches off for the **handbrake**, a **sudden impact**, staying **far below the minimum speed** (stuck in traffic), **leaving the driver seat**, **scenes**, **AutoDrive**, and vehicle **quickhacks** or **remote control**.
- It **brakes lightly** downhill or after lowering the cruise speed so the car does not run away, up to the maximum cruise braking. Turn *Brake to hold speed* off to only coast.
- Steering, the Sport and Gentle keys, and the horn keep working while cruising.
- Switching cruise control on switches the [speed limiter](#speed-limiter) off.
- A short on-screen message and a click confirm each change. Both can be turned off.

Cruise control works with keyboard and controller alike.

## Speed Limiter

The speed limiter keeps the car from going faster than a limit you set. Drive as usual: below the limit the throttle is yours, and close to it the throttle eases off, so the car settles on the limit instead of passing it.

- **Switch it on** with the speed limiter key. The limit is your current speed rounded up to a step of 5, so switching it on never slows you down: 57 becomes 60, and 60 stays 60. Below the minimum speed, for example parked, it uses the last limit, or the *Default limit* (60) if you have not set one since loading the game.
- **Change the limit** with set speed up and down, in steps of 5. A limit below your speed slows the car down smoothly.
- **Pass the limit** by holding the **Sport Mode key**, for example to overtake. When you let go, the car slows back down to the limit. Sport mode switched on with a tap does not lift the limit. Turn off *Sport key passes the limit* for a strict limit.
- **Braking, the handbrake and crashes** leave it on, since it only ever holds the throttle back. During scenes, AutoDrive, vehicle quickhacks and remote control it waits, and it carries on afterwards.
- It **brakes lightly** downhill, after lowering the limit, and after passing it with the Sport key, up to the maximum limiter braking. Turn *Brake to stay at the limit* off to only coast.
- It **stays on when you leave the car** and limits the next car you drive, with a short reminder when you get in. Turn off *Stay on after leaving the car* to switch it off when you leave the driver seat. Loading a save always starts with it off.
- Switching the speed limiter on switches cruise control off, and the other way around.
- A short on-screen message and a click confirm each change. Both can be turned off.

The speed limiter works with keyboard and controller alike.

## On-Screen Indicators

While you drive, whatever is on is listed with the game's own driving hints (change camera, draw weapon), each with the key that switches it off:

- **Cruise 60** — cruise control holding 60, in your [speed units](#speed-units).
- **Limit 60** — the speed limiter set to 60.
- **Sport mode**, **Gentle mode** — a mode switched on with a tap. A mode that is only on while you hold its key is not listed, the key under your finger says it already.

The list follows the game's HUD, so it disappears when the HUD does, and while you are not really driving (scenes, AutoDrive, quickhacks, remote control). Turn it off with *Show indicators* in General.

## Speed Units

The game's speedometers show a number that runs well above the true speed, and cruise control and the speed limiter use the number you actually see. Pick the one that matches the display you read in **General > Speed units**:

- **Car dashboard** (default) — the display inside the car. It shows the same number whatever the game's metric or imperial setting.
- **HUD speedometer** — the third person speedometer, which follows the game's *Speedometer units* setting.
- **True km/h** and **True mph** — the real speed, which is lower than either speedometer.

## Keyboard, Controller and Motorcycles

- **Keyboard only by default.** Triggers and sticks are already analog, so throttle, brake and steering levels only apply to keyboard driving. Turn on *Shape controller input too* to apply them to a controller as well. Cruise control and the speed limiter work with any device.
- **Controller buttons** can't be rebound in Mod Settings. To use one, add a `<button id="IK_Pad_..."/>` line to the matching mapping in `r6/input/ImmersiveDriving.xml`.
- **Motorcycles** are supported, and can be switched off separately from cars.
- **Lean keys.** In the vanilla game Left Shift and Left Ctrl lean forward and back (motorcycles, and cars in the air), and the game scales steering down while a lean key is held. While the Sport or Gentle key is held on the keyboard, the mod restores full steering and drops that lean, so Sport on Left Shift steers fully and does not lean. A mode switched on with a toggle key leaves leaning alone once the key is released.

## In-Game Configuration

Open **Mods > Drive Modes and Cruise Control** from the main menu or the pause menu (this needs Mod Settings). Changes apply as soon as you accept them, including while driving, with no reload. Without Mod Settings the mod runs with the default values and keys.

## Settings Reference

### General

| Setting | Default | Meaning |
| --- | --- | --- |
| Enable Drive Modes and Cruise Control | On | Master switch. When off, driving is completely vanilla. |
| Speed units | Car dashboard | Units for cruise control speeds and speed limits, see [Speed Units](#speed-units). |
| Minimum speed | 20 km/h | The lowest cruise speed and speed limit, as a true speed. Cruise control only switches on above it, and switches off when the car stays far below it. Below it the speed limiter key uses the last or default limit. |
| Show indicators | On | Keeps cruise control, the speed limiter, and a mode left on by a toggle key listed with the game's driving hints for as long as they are on, see [On-Screen Indicators](#on-screen-indicators). |
| Show messages | On | Short on-screen messages when cruise control or the speed limiter changes, or a toggle key switches a mode on or off. |
| Play sounds | On | A soft click when cruise control, the speed limiter, or a mode on a toggle key switches on or off. |
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
| Speed change rate | 5 km/h per second | How quickly the car reaches a new cruise speed. |
| Responsiveness | 100% | How firmly the speed is held. Lower feels softer, higher reacts faster to hills. |
| After using the pedals | Resume set speed | *Resume set speed* returns to the cruise speed after accelerating. *Use new speed* keeps the speed you let go at. |
| Brake cancels cruise control | On | When off, braking only pauses cruise control. |
| Handbrake cancels cruise control | On | |
| Crashes cancel cruise control | On | Switch off after a sudden impact. |
| Brake to hold speed | On | Brake lightly downhill or after lowering the cruise speed. When off, the car only coasts. |
| Maximum cruise braking | 30% | Most brake cruise control uses. |
| Maximum cruise throttle | 100% | Lower values climb hills and reach a higher cruise speed more gently. |

### Speed Limiter

| Setting | Default | Meaning |
| --- | --- | --- |
| Enable speed limiter | On | |
| Default limit | 60 | The limit, in the selected speed units, when the speed limiter key is pressed below the minimum speed before any limit was set since loading the game. |
| Stay on after leaving the car | On | Keep limiting the next car you drive. When off, leaving the driver seat switches the speed limiter off. |
| Sport key passes the limit | On | Holding the Sport Mode key lifts the limit until you let go. |
| Brake to stay at the limit | On | Brake lightly downhill, after lowering the limit, or after passing it. When off, the car only coasts. |
| Maximum limiter braking | 30% | Most brake the speed limiter uses. |

### Key Bindings

The keys themselves are listed in [Default Key Bindings](#default-key-bindings).

| Setting | Default | Meaning |
| --- | --- | --- |
| Sport Mode key | Tap or hold | *Hold*: Sport mode while the key is held. *Toggle*: press to switch Sport mode on, press again to switch it off. *Tap or hold*: a short tap toggles, holding gives Sport mode only while held, or the Default levels while held when Sport mode is on. |
| Gentle Mode key | Tap or hold | *Hold*: Gentle mode while the key is held. *Toggle*: press to switch Gentle mode on, press again to switch it off. *Tap or hold*: a short tap toggles, holding gives Gentle mode only while held, or the Default levels while held when Gentle mode is on. |
| First key does | Cruise control | What the first key switches, on a short press and on a long press of about half a second: *Cruise control*, *Speed limiter*, *Cruise control, speed limiter on long press*, *Speed limiter, cruise control on long press*, or *Nothing*. |
| Second key does | Speed limiter | The same for the second key. A key that uses a long press acts when you let go, cruise control then starts from the speed you had when you pressed it, and a short press switches off what the long press put on. |
| Set speed keys | Start cruise control | What set speed up and down do while cruise control and the speed limiter are both off. *Start cruise control*: up switches it back on at the last cruise speed, down at the current speed. *Start speed limiter*: up switches it back on at the last limit, down at the current speed rounded up. *Do nothing*: they only change a speed that is already on. |

### Advanced

| Setting | Default | Meaning |
| --- | --- | --- |
| Debug logging | Off | Writes driving values, key presses, and cruise speed and speed limit changes to the log. |

## Logs and Debug Logging

The mod writes its log to `red4ext/logs/ImmersiveDriving-*.log` in the game folder. With Mod Organizer 2 the logs land in the MO2 `overwrite` folder instead, for example `overwrite/red4ext/logs/`.

A healthy start logs `Hooked vehicle::BaseObject::UpdateVehicleCameraInput` and `Registered native functions`. With **Advanced > Debug logging** on, the log also shows once per second the speed, the inputs the game computed and what the mod wrote, plus every press of the mod's keys and every cruise speed and speed limit change. That log is the most useful thing to attach to a bug report.
