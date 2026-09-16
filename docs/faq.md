# FAQ and Troubleshooting

Common questions and fixes. For how the modes and cruise control work and the full list of settings, see the [Usage and Configuration Guide](README.md); for install and requirements see the [main README](../README.md); for what changed in each release see the [Changelog](changelog.md).

Click a question to expand its answer.

## Contents

- [Getting It Working](#getting-it-working)
- [Cruise Control](#cruise-control)
- [Keys and Controllers](#keys-and-controllers)
- [Compatibility](#compatibility)
- [Settings, Logs and Uninstalling](#settings-logs-and-uninstalling)
- [Reporting a Bug](#reporting-a-bug)

## Getting It Working

<details>
  <summary>Nothing changes while driving</summary>
&#10240;

The plugin is probably not loading. Open the RED4ext log (`red4ext/logs/red4ext-*.log`, or the MO2 `overwrite` folder when using Mod Organizer 2):

- If it says the plugin *uses RED4ext.SDK ... which is not supported* or does not list `ImmersiveDriving` at all, update RED4ext and check that the game is patch 2.31.
- If it lists `ImmersiveDriving ... has been loaded`, open `red4ext/logs/ImmersiveDriving-*.log`. It should say `Hooked vehicle::BaseObject::UpdateVehicleCameraInput` and `Registered native functions`. If the hook is missing, the game version does not match.

Also check that **Enable Drive Modes and Cruise Control**, and **Cars** or **Motorcycles**, are on in the General settings. On a controller the levels only apply with *Shape controller input too* on, see [Keys and Controllers](#keys-and-controllers).

</details>

<details>
  <summary>There is no Drive Modes and Cruise Control entry in the Mods menu</summary>
&#10240;

The Mods menu comes from [Mod Settings](https://www.nexusmods.com/cyberpunk2077/mods/4885), which needs [ArchiveXL](https://www.nexusmods.com/cyberpunk2077/mods/4198). Install both. The menu is in the main menu and the pause menu. Without Mod Settings the mod still works with its default values and keys.

If Mod Settings is installed but the entry is missing, the mod's scripts did not compile, usually because the plugin did not load (see *Nothing changes while driving* above). Check `r6/logs/redscript_rCURRENT.log` for errors.

</details>

<details>
  <summary>The mod's keys do nothing</summary>
&#10240;

The keys come from [Input Loader](https://www.nexusmods.com/cyberpunk2077/mods/4575). Check that it is installed and that `red4ext/logs/input_loader.log` lists `ImmersiveDriving.xml`. Mods that replace the game's `r6/config/inputUserMappings.xml` or `inputContexts.xml` stop Input Loader from adding any mod's keys, so remove them.

The keys are only active while you drive. To see exactly which key presses reach the mod, turn on **Advanced > Debug logging**; every press of the mod's keys is written to the log.

</details>

<details>
  <summary>I changed a setting but the car still drives the same</summary>
&#10240;

Settings apply as soon as you accept them in Mod Settings, including while driving. If a value seems to have no effect:

- Check that you are driving with the **keyboard**, or that *Shape controller input too* is on.
- Check that no mode is active: no mode key held, and no mode switched on with a toggle key. Sport Mode uses the Sport values and Gentle Mode the Gentle values.
- Steering at speed is also reduced by **speed-sensitive steering**.

Your saved values override the defaults of a new version. Use **Defaults** in Mod Settings to go back to the current defaults.

</details>

<details>
  <summary>The game crashes at startup after I edited the input file</summary>
&#10240;

Some keys crash the game when they are set as defaults in `r6/input/ImmersiveDriving.xml`; the mouse wheel does on patch 2.31. Restore the file from the release archive, and bind those keys in **Key Bindings** in Mod Settings instead, which works.

</details>

## Cruise Control

<details>
  <summary>The cruise speed does not match my speedometer</summary>
&#10240;

The game's speedometers show a number that is well above the true speed, and the car's own display and the HUD speedometer can even show different units. Set **General > Speed units** to the display you read:

- **Car dashboard** for the display inside the car (it ignores the game's metric or imperial setting).
- **HUD speedometer** for the third person speedometer (follows the game's *Speedometer units* setting).
- **True km/h** or **True mph** only if you want the real speed, which reads lower than both speedometers.

While cruising, the speedometer can move by one up or down as cruise control corrects for hills.

</details>

<details>
  <summary>Why are cruise speeds always steps of 5?</summary>
&#10240;

So the numbers stay round, like 50, 55, 60. Switching on at 57 sets 55, and cruise speed up from there gives 60. The steps are counted in the selected speed units, so with *Car dashboard* they match the car's display.

</details>

<details>
  <summary>Cruise control switches itself off</summary>
&#10240;

It switches off, with a short message, when you:

- **Brake** (turn off *Brake cancels cruise control* to make braking only pause it),
- pull the **handbrake** or **crash** into something (both can be turned off),
- stay **far below the minimum speed** for a moment, for example stuck in traffic,
- **leave the driver seat**, or a **scene**, **AutoDrive**, a vehicle **quickhack** or **remote control** takes over the car.

Accelerating never switches it off. When you let go of the throttle, cruise control returns to the cruise speed.

</details>

<details>
  <summary>Cruise control will not switch on</summary>
&#10240;

- The car has to be above the **minimum speed** (20 km/h true speed by default). A message tells you when you are too slow.
- **Enable cruise control** has to be on.
- It stays silent while you are not really driving: as a passenger, in a scene, with AutoDrive, or while the car is quickhacked.

</details>

## Keys and Controllers

<details>
  <summary>Can I use the mouse wheel for cruise speed?</summary>
&#10240;

Yes. Bind **Cruise speed up** and **Cruise speed down** to the mouse wheel in **Key Bindings**. Keep in mind that the wheel also zooms the vehicle camera in the vanilla game.

Do not put the mouse wheel into `r6/input/ImmersiveDriving.xml` as the default key; see *The game crashes at startup after I edited the input file* in [Getting It Working](#getting-it-working).

</details>

<details>
  <summary>Does it work with a controller?</summary>
&#10240;

Cruise control works with any input device. The throttle, brake and steering levels only apply to keyboard driving by default, because triggers and sticks are already analog; turn on **General > Shape controller input too** to apply them to a controller as well.

Controller buttons cannot be rebound in Mod Settings. To use one for a mod key, add a `<button id="IK_Pad_..."/>` line to the matching mapping in `r6/input/ImmersiveDriving.xml`.

</details>

<details>
  <summary>Holding Shift or Ctrl used to lean my motorcycle</summary>
&#10240;

In the vanilla game Left Shift and Left Ctrl lean forward and back, and the game reduces steering to about 71% while a lean key is held with A or D. Because Left Shift is the default Sport Mode key, the mod restores full steering and drops the lean while the Sport Mode or Gentle Mode key is held on the keyboard. To keep leaning with Shift, bind Sport Mode to another key in **Key Bindings**.

</details>

## Compatibility

<details>
  <summary>Is it compatible with Immersive Driving and other driving mods?</summary>
&#10240;

- **Immersive Driving** (mod 5293) replaces the game's input files, which stops Input Loader from adding this mod's keys. Remove it.
- Other mods that replace `r6/config/inputUserMappings.xml` or `inputContexts.xml` have the same problem.
- Plugins that also rewrite the car's driving inputs every frame, such as steering wheel plugins or flight mods while flying, may combine with this mod in unexpected ways. Disable the overlapping features in one of them.
- Traffic, AutoDrive, remote control, quickhacks and scripted scenes are left alone.

</details>

<details>
  <summary>Will it break after a game or framework update?</summary>
&#10240;

- **Game patch:** the plugin only loads on patch 2.31. On another patch RED4ext skips it, and its scripts are not compiled, so driving is simply vanilla until an updated version is released.
- **RED4ext, redscript, Input Loader or Mod Settings:** tested with the versions listed in the [main README](../README.md#tested-with). Newer versions usually keep working; if the mod stops loading after an update, check the logs described in *Nothing changes while driving* under [Getting It Working](#getting-it-working).

</details>

## Settings, Logs and Uninstalling

<details>
  <summary>Where are the settings and logs?</summary>
&#10240;

- Settings are saved by Mod Settings in `red4ext/plugins/mod_settings/user.ini`, under `[ImmersiveDrivingSettings]`.
- The mod's log is `red4ext/logs/ImmersiveDriving-*.log`, and the script compiler's log is `r6/logs/redscript_rCURRENT.log`.
- With Mod Organizer 2, files the game writes land in the MO2 `overwrite` folder, and `user.ini` can end up in the Mod Settings mod folder or the `overwrite` folder.

Turn on **Advanced > Debug logging** before reproducing a problem; that log is the best thing to attach to a bug report, see [Reporting a Bug](#reporting-a-bug).

</details>

<details>
  <summary>How do I uninstall it?</summary>
&#10240;

Remove the mod in your mod manager, or delete `red4ext/plugins/ImmersiveDriving` and `r6/input/ImmersiveDriving.xml` from the game folder. It does not change saves, so it can be removed at any time.

</details>

## Reporting a Bug

A debug log shows exactly what the mod sees and does while you drive, so it usually answers the question on the first try. It takes a few minutes:

### 1. Turn on debug logging

In the main menu or the pause menu, open **Mods > Drive Modes and Cruise Control > Advanced** and turn on **Debug logging**. It applies immediately, no restart needed.

### 2. Reproduce the problem

Get in the vehicle where the problem happens and do the thing that goes wrong. Keep it short, a minute or two is plenty, and remember what you pressed. For example, for a problem with the modes: hold accelerate for a few seconds with no mode, then in Gentle, then in Sport.

With debug logging on, the log gets one line per second while you drive, plus every press of the mod's keys, every cruise control change, and every time you get in or out of the driver seat.

### 3. Find the log

The log is in the game folder under `red4ext/logs`, named `immersivedriving-<date>-<time>.log`. Each game launch starts a new file, so take the newest one.

| Mod manager | Folder |
| --- | --- |
| Vortex or manual install | `<game folder>/red4ext/logs/` |
| Mod Organizer 2 | `<MO2 folder>/overwrite/red4ext/logs/` |

The game folder is the one that contains `bin`, `r6` and `red4ext`. In Steam: right-click Cyberpunk 2077 > **Manage** > **Browse local files**. In GOG Galaxy: **Manage installation** > **Show folder**.

### 4. Post it on Pastebin

1. Open the log in Notepad, select everything (Ctrl+A) and copy it (Ctrl+C).
2. Go to [pastebin.com](https://pastebin.com), paste it into the big text box, and set **Paste Exposure** to *Unlisted*.
3. Click **Create New Paste** and copy the link from the address bar.

If Pastebin says the paste is too large, keep only the start of the log and the part around the problem, or launch the game again and reproduce it in a shorter session.

### 5. Post the report

Post the Pastebin link in the mod's **Bugs** tab on Nexus, together with:

- What you did and what happened, including which vehicle and which keys.
- The mod version, and your mod manager.
- Other driving, vehicle or input mods you use.

Turn **Debug logging** off again afterwards; it is not needed for normal play.
