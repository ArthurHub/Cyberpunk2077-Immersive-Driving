## v1.3

- Indicators: Cruise control, the speed limiter, and a mode left on by a toggle key now stay listed with the game's driving hints (change camera, draw weapon) for as long as they are on, each with the key that switches it off. Turn them off with Show indicators in General.
- Keys: What each of the two keys does is now a setting. A key can switch cruise control or the speed limiter on a short press, the other one on a long press of about half a second, or nothing at all, so both fit on one key and the other key is free.
- Keys: With a long press in use, that key acts when it is released, cruise control starts from the speed you had when you pressed it, and a short press switches off what the long press put on.
- Config: Cruise control on / off and Speed limiter on / off are now First key and Second key in Key Bindings, with First key does and Second key does next to them. The keys themselves and their defaults are unchanged.

## v1.2

- Speed Limiter: A new speed limiter keeps the car from going faster than a set limit. Mouse 4 switches it on at the current speed rounded up to a step of 5, or off, and set speed up and down change the limit. The throttle eases off before the limit, and the car brakes lightly downhill. Braking, the handbrake and crashes leave it on.
- Speed Limiter: Holding the Sport Mode key passes the limit, for example to overtake, and the car returns to the limit when you let go (Sport key passes the limit).
- Speed Limiter: Stays on after leaving the car by default (Stay on after leaving the car), and uses a Default limit (60) when switched on while parked.
- Speed Limiter: Only one of cruise control and the speed limiter is on at a time.
- Keys: Cruise speed up and down are now Set speed up and down, and change whichever of cruise control and the speed limiter is on. Set speed keys chooses what they start while both are off: cruise control (default), the speed limiter, or nothing.
- Config: Minimum speed moved from Cruise Control to General, because it is also the lowest speed limit.
- Fix: AutoDrive was not detected, so cruise control could switch on during AutoDrive and the Sport and Gentle keys announced modes that did nothing. The mod now stays out of AutoDrive, and the speed limiter waits until you drive again.

## v1.1

- Drive Modes: The Sport and Gentle keys can each work as Hold, Toggle (press to switch the mode on, press again to switch it off), or Tap or hold (a short tap toggles, holding flips the mode only while held without toggling), set in Key Bindings. Tap or hold is the new default. Toggling one mode on switches the other toggled mode off, and holding a key overrides a toggled mode while held.
- Fix: Switching back to the driver seat with Auto Drive Enhanced left the mod inactive until you got out of the car.

## v1.0

- Initial release.
- Drive Modes: Throttle, brake and steering each have a Default level, a Sport level while holding the Sport key (Left Shift), and a Gentle level while holding the Gentle key (Left Alt), all adjustable from 0 to 100%.
- Steering: Smooth steering eases in while you hold a direction and lets go instantly. Speed-sensitive steering reduces Default and Gentle steering at high speed.
- Cruise Control: Holds a set speed on flat roads, uphill and downhill, with speed up and down in steps of 5, resume, and a smooth speed change rate.
- Cruise Control: Accelerating overtakes and returns to the cruise speed, or adopts the new speed. Braking, the handbrake, crashes, getting stuck in traffic, scenes, AutoDrive, quickhacks and leaving the driver seat switch it off.
- Cruise Control: Speeds match the car's dashboard, the HUD speedometer, or true km/h or mph.
- Lean keys: The Sport and Gentle keys no longer lean the vehicle or reduce steering when they share the vanilla lean keys.
- Config: Every value and key binding is configurable in-game through Mod Settings and applies immediately, with optional on-screen messages, sounds and debug logging.
- Controllers: Levels apply to keyboard driving only by default, cruise control works with any input device.
