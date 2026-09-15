// Functions registered by ImmersiveDriving.dll (src/plugin/Natives.cpp). Keep both lists in sync.
// Speeds are in m/s.

public native func ImmersiveDriving_GetVersion() -> String;

// True once the vehicle hook is installed.
public native func ImmersiveDriving_IsReady() -> Bool;

// One-line diagnostic summary of the native state.
public native func ImmersiveDriving_GetStatus() -> String;

// Writes a line to red4ext/logs/ImmersiveDriving-*.log.
public native func ImmersiveDriving_Log(message: String) -> Void;

// Settings, pushed by name (see ImmersiveDrivingSettings.Push). Return false for unknown names.
public native func ImmersiveDriving_SetBool(name: CName, value: Bool) -> Bool;
public native func ImmersiveDriving_SetFloat(name: CName, value: Float) -> Bool;
public native func ImmersiveDriving_SetInt(name: CName, value: Int32) -> Bool;

// Driving context. kind: 1 car, 2 motorcycle, 3 unsupported.
public native func ImmersiveDriving_SetPlayerVehicle(vehicle: ref<VehicleObject>, kind: Int32) -> Void;
public native func ImmersiveDriving_ClearPlayerVehicle() -> Void;
public native func ImmersiveDriving_SetDrivingAllowed(allowed: Bool) -> Void;
public native func ImmersiveDriving_SetUsingKeyboard(usingKeyboard: Bool) -> Void;
public native func ImmersiveDriving_SetGentle(active: Bool) -> Void;
// The Sport key: full throttle, brake and steering while held.
public native func ImmersiveDriving_SetSport(active: Bool) -> Void;

// Cruise control. EngageCruise returns 0 engaged, 1 disabled, 2 not driving, 3 too slow, 4 not ready.
public native func ImmersiveDriving_EngageCruise(targetSpeed: Float) -> Int32;
public native func ImmersiveDriving_SetCruiseTarget(targetSpeed: Float) -> Bool;
public native func ImmersiveDriving_CancelCruise() -> Bool;
public native func ImmersiveDriving_IsCruiseActive() -> Bool;
public native func ImmersiveDriving_GetCruiseTarget() -> Float;
public native func ImmersiveDriving_GetLastCruiseTarget() -> Float;
public native func ImmersiveDriving_GetSpeed() -> Float;

// Cruise changes the scripts did not request: 0 none, 1 target adopted after using the pedals, 2 cancelled by brake,
// 3 by handbrake, 4 by a collision, 5 too slow, 6 unavailable (left the seat, scene, AutoDrive, disabled).
public native func ImmersiveDriving_PopCruiseEvent() -> Int32;
