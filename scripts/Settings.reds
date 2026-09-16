// In-game settings, shown in Mod Settings under "Drive Modes and Cruise Control".
// Field names match the native Config (src/core/Config.h), and the key binding names match the
// overridableUI names in input/ImmersiveDriving.xml.

// Mod Settings saves enums by name, so new values are appended.
enum ImmersiveDrivingSpeedUnit {
  Dashboard = 0,
  Kmh = 1,
  Mph = 2,
  HudSpeedometer = 3
}

enum ImmersiveDrivingAfterOverride {
  ResumeSetSpeed = 0,
  UseNewSpeed = 1
}

enum ImmersiveDrivingKeyMode {
  Hold = 0,
  Toggle = 1,
  TapOrHold = 2
}

// What the set speed up and down keys do while cruise control and the speed limiter are both off.
enum ImmersiveDrivingSetSpeedKeys {
  StartCruise = 0,
  StartLimiter = 1,
  Nothing = 2
}

public class ImmersiveDrivingSettings extends IScriptable {

  // Told about changes that stay in the scripts (key modes). Not a Mod Settings option.
  public let system: wref<ImmersiveDrivingSystem>;

  // ---------------------------------------------------------------------------------------------------------------------
  // General

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "General")
  @runtimeProperty("ModSettings.category.order", "0")
  @runtimeProperty("ModSettings.displayName", "Enable Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.description", "Master switch. When off, driving is completely vanilla.")
  public let enabled: Bool = true;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "General")
  @runtimeProperty("ModSettings.category.order", "0")
  @runtimeProperty("ModSettings.displayName", "Speed units")
  @runtimeProperty("ModSettings.description", "Units for cruise control speeds and speed limits, which are always whole steps of 5. Car dashboard matches the speed on the car's own display. HUD speedometer matches the third person speedometer, which follows the game's metric or imperial setting. Both use the game's speedometer numbers, which run higher than the true speed.")
  @runtimeProperty("ModSettings.displayValues.Dashboard", "Car dashboard")
  @runtimeProperty("ModSettings.displayValues.HudSpeedometer", "HUD speedometer")
  @runtimeProperty("ModSettings.displayValues.Kmh", "True km/h")
  @runtimeProperty("ModSettings.displayValues.Mph", "True mph")
  public let speedUnit: ImmersiveDrivingSpeedUnit = ImmersiveDrivingSpeedUnit.Dashboard;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "General")
  @runtimeProperty("ModSettings.category.order", "0")
  @runtimeProperty("ModSettings.displayName", "Minimum speed (km/h)")
  @runtimeProperty("ModSettings.description", "The lowest cruise speed and speed limit, as a true speed. Cruise control only switches on above it, and switches off when the car stays far below it, for example stuck in traffic. Below it the speed limiter key uses the last or default limit. 20 km/h is about 12 mph.")
  @runtimeProperty("ModSettings.min", "5.0")
  @runtimeProperty("ModSettings.max", "100.0")
  @runtimeProperty("ModSettings.step", "5.0")
  public let cruiseMinKmh: Float = 20.0;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "General")
  @runtimeProperty("ModSettings.category.order", "0")
  @runtimeProperty("ModSettings.displayName", "Show messages")
  @runtimeProperty("ModSettings.description", "Short on-screen messages when cruise control or the speed limiter changes, or a toggle key switches a mode on or off.")
  public let showMessages: Bool = true;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "General")
  @runtimeProperty("ModSettings.category.order", "0")
  @runtimeProperty("ModSettings.displayName", "Play sounds")
  @runtimeProperty("ModSettings.description", "A soft click when cruise control, the speed limiter, or a mode on a toggle key switches on or off.")
  public let playSounds: Bool = true;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "General")
  @runtimeProperty("ModSettings.category.order", "0")
  @runtimeProperty("ModSettings.displayName", "Cars")
  @runtimeProperty("ModSettings.description", "Use the mod in cars, vans and trucks.")
  public let applyToCars: Bool = true;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "General")
  @runtimeProperty("ModSettings.category.order", "0")
  @runtimeProperty("ModSettings.displayName", "Motorcycles")
  @runtimeProperty("ModSettings.description", "Use the mod on motorcycles.")
  public let applyToBikes: Bool = true;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "General")
  @runtimeProperty("ModSettings.category.order", "0")
  @runtimeProperty("ModSettings.displayName", "Shape controller input too")
  @runtimeProperty("ModSettings.description", "Throttle, brake and steering levels only apply to keyboard driving by default, because triggers and sticks are already analog. Cruise control always works.")
  public let applyToGamepad: Bool = false;

  // ---------------------------------------------------------------------------------------------------------------------
  // Throttle, brake and steering: default, in Sport mode, and in Gentle mode. Sport wins when both are active.

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Throttle, Brake and Steering")
  @runtimeProperty("ModSettings.category.order", "1")
  @runtimeProperty("ModSettings.displayName", "Default throttle (%)")
  @runtimeProperty("ModSettings.description", "Throttle when holding accelerate with neither Sport nor Gentle mode active.")
  @runtimeProperty("ModSettings.min", "0")
  @runtimeProperty("ModSettings.max", "100")
  @runtimeProperty("ModSettings.step", "5")
  public let throttleNormalPct: Int32 = 60;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Throttle, Brake and Steering")
  @runtimeProperty("ModSettings.category.order", "1")
  @runtimeProperty("ModSettings.displayName", "Sport throttle (%)")
  @runtimeProperty("ModSettings.description", "Throttle when holding accelerate in Sport mode.")
  @runtimeProperty("ModSettings.min", "0")
  @runtimeProperty("ModSettings.max", "100")
  @runtimeProperty("ModSettings.step", "5")
  public let throttleSportPct: Int32 = 100;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Throttle, Brake and Steering")
  @runtimeProperty("ModSettings.category.order", "1")
  @runtimeProperty("ModSettings.displayName", "Gentle throttle (%)")
  @runtimeProperty("ModSettings.description", "Throttle when holding accelerate in Gentle mode.")
  @runtimeProperty("ModSettings.min", "0")
  @runtimeProperty("ModSettings.max", "100")
  @runtimeProperty("ModSettings.step", "5")
  public let throttleGentlePct: Int32 = 25;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Throttle, Brake and Steering")
  @runtimeProperty("ModSettings.category.order", "1")
  @runtimeProperty("ModSettings.displayName", "Default brake (%)")
  @runtimeProperty("ModSettings.description", "Brake (and reverse throttle) when holding brake with neither Sport nor Gentle mode active.")
  @runtimeProperty("ModSettings.min", "0")
  @runtimeProperty("ModSettings.max", "100")
  @runtimeProperty("ModSettings.step", "5")
  public let brakeNormalPct: Int32 = 50;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Throttle, Brake and Steering")
  @runtimeProperty("ModSettings.category.order", "1")
  @runtimeProperty("ModSettings.displayName", "Sport brake (%)")
  @runtimeProperty("ModSettings.description", "Brake (and reverse throttle) when holding brake in Sport mode.")
  @runtimeProperty("ModSettings.min", "0")
  @runtimeProperty("ModSettings.max", "100")
  @runtimeProperty("ModSettings.step", "5")
  public let brakeSportPct: Int32 = 100;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Throttle, Brake and Steering")
  @runtimeProperty("ModSettings.category.order", "1")
  @runtimeProperty("ModSettings.displayName", "Gentle brake (%)")
  @runtimeProperty("ModSettings.description", "Brake (and reverse throttle) when holding brake in Gentle mode.")
  @runtimeProperty("ModSettings.min", "0")
  @runtimeProperty("ModSettings.max", "100")
  @runtimeProperty("ModSettings.step", "5")
  public let brakeGentlePct: Int32 = 25;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Throttle, Brake and Steering")
  @runtimeProperty("ModSettings.category.order", "1")
  @runtimeProperty("ModSettings.displayName", "Default steering (%)")
  @runtimeProperty("ModSettings.description", "Steering when holding left or right with neither Sport nor Gentle mode active.")
  @runtimeProperty("ModSettings.min", "0")
  @runtimeProperty("ModSettings.max", "100")
  @runtimeProperty("ModSettings.step", "5")
  public let steerNormalPct: Int32 = 75;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Throttle, Brake and Steering")
  @runtimeProperty("ModSettings.category.order", "1")
  @runtimeProperty("ModSettings.displayName", "Sport steering (%)")
  @runtimeProperty("ModSettings.description", "Steering when holding left or right in Sport mode. Speed-sensitive steering does not reduce it.")
  @runtimeProperty("ModSettings.min", "0")
  @runtimeProperty("ModSettings.max", "100")
  @runtimeProperty("ModSettings.step", "5")
  public let steerSportPct: Int32 = 100;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Throttle, Brake and Steering")
  @runtimeProperty("ModSettings.category.order", "1")
  @runtimeProperty("ModSettings.displayName", "Gentle steering (%)")
  @runtimeProperty("ModSettings.description", "Steering when holding left or right in Gentle mode, for gentle curves and lane changes.")
  @runtimeProperty("ModSettings.min", "0")
  @runtimeProperty("ModSettings.max", "100")
  @runtimeProperty("ModSettings.step", "5")
  public let steerGentlePct: Int32 = 50;

  // ---------------------------------------------------------------------------------------------------------------------
  // Advanced steering

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Advanced Steering")
  @runtimeProperty("ModSettings.category.order", "2")
  @runtimeProperty("ModSettings.displayName", "Smooth steering")
  @runtimeProperty("ModSettings.description", "Steering builds up while holding left or right, so short taps make small corrections. Letting go is always instant.")
  public let steeringSmoothing: Bool = true;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Advanced Steering")
  @runtimeProperty("ModSettings.category.order", "2")
  @runtimeProperty("ModSettings.displayName", "Steer-in time (s)")
  @runtimeProperty("ModSettings.description", "Seconds to reach the steering level while holding a direction.")
  @runtimeProperty("ModSettings.min", "0.0")
  @runtimeProperty("ModSettings.max", "2.0")
  @runtimeProperty("ModSettings.step", "0.05")
  @runtimeProperty("ModSettings.dependency", "steeringSmoothing")
  public let steerRiseSec: Float = 0.3;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Advanced Steering")
  @runtimeProperty("ModSettings.category.order", "2")
  @runtimeProperty("ModSettings.displayName", "Speed-sensitive steering")
  @runtimeProperty("ModSettings.description", "Less Default and Gentle steering at high speed, for stable lane changes and long highway curves.")
  public let speedSensitiveSteering: Bool = true;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Advanced Steering")
  @runtimeProperty("ModSettings.category.order", "2")
  @runtimeProperty("ModSettings.displayName", "Full steering below (km/h)")
  @runtimeProperty("ModSettings.description", "Below this true speed steering is never reduced. 40 km/h is about 25 mph.")
  @runtimeProperty("ModSettings.min", "0.0")
  @runtimeProperty("ModSettings.max", "150.0")
  @runtimeProperty("ModSettings.step", "5.0")
  @runtimeProperty("ModSettings.dependency", "speedSensitiveSteering")
  public let fullSteerBelowKmh: Float = 40.0;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Advanced Steering")
  @runtimeProperty("ModSettings.category.order", "2")
  @runtimeProperty("ModSettings.displayName", "High speed (km/h)")
  @runtimeProperty("ModSettings.description", "True speed at which steering reaches its high speed level. 140 km/h is about 87 mph.")
  @runtimeProperty("ModSettings.min", "40.0")
  @runtimeProperty("ModSettings.max", "300.0")
  @runtimeProperty("ModSettings.step", "5.0")
  @runtimeProperty("ModSettings.dependency", "speedSensitiveSteering")
  public let highSpeedKmh: Float = 140.0;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Advanced Steering")
  @runtimeProperty("ModSettings.category.order", "2")
  @runtimeProperty("ModSettings.displayName", "Steering at high speed (%)")
  @runtimeProperty("ModSettings.description", "Steering at and above the high speed, as a percentage of the Default or Gentle steering level.")
  @runtimeProperty("ModSettings.min", "10")
  @runtimeProperty("ModSettings.max", "100")
  @runtimeProperty("ModSettings.step", "5")
  @runtimeProperty("ModSettings.dependency", "speedSensitiveSteering")
  public let highSpeedSteerPct: Int32 = 65;

  // ---------------------------------------------------------------------------------------------------------------------
  // Cruise control

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Cruise Control")
  @runtimeProperty("ModSettings.category.order", "3")
  @runtimeProperty("ModSettings.displayName", "Enable cruise control")
  @runtimeProperty("ModSettings.description", "Hold a set speed without touching the pedals. Set speeds are whole steps of 5 in the selected speed units. Steering keeps working.")
  public let cruiseEnabled: Bool = true;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Cruise Control")
  @runtimeProperty("ModSettings.category.order", "3")
  @runtimeProperty("ModSettings.displayName", "Speed change rate (km/h per second)")
  @runtimeProperty("ModSettings.description", "How quickly the car speeds up or slows down to a new set speed.")
  @runtimeProperty("ModSettings.min", "1.0")
  @runtimeProperty("ModSettings.max", "30.0")
  @runtimeProperty("ModSettings.step", "1.0")
  @runtimeProperty("ModSettings.dependency", "cruiseEnabled")
  public let cruiseChangeRateKmhps: Float = 5.0;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Cruise Control")
  @runtimeProperty("ModSettings.category.order", "3")
  @runtimeProperty("ModSettings.displayName", "Responsiveness (%)")
  @runtimeProperty("ModSettings.description", "How firmly the speed is held. Lower feels softer, higher reacts faster to hills.")
  @runtimeProperty("ModSettings.min", "25")
  @runtimeProperty("ModSettings.max", "200")
  @runtimeProperty("ModSettings.step", "25")
  @runtimeProperty("ModSettings.dependency", "cruiseEnabled")
  public let cruiseResponsivenessPct: Int32 = 100;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Cruise Control")
  @runtimeProperty("ModSettings.category.order", "3")
  @runtimeProperty("ModSettings.displayName", "After using the pedals")
  @runtimeProperty("ModSettings.description", "Resume set speed: like a real car, cruise control returns to the set speed after you accelerate past it. Use new speed: the speed at which you let go, rounded to a step of 5, becomes the new set speed.")
  @runtimeProperty("ModSettings.displayValues.ResumeSetSpeed", "Resume set speed")
  @runtimeProperty("ModSettings.displayValues.UseNewSpeed", "Use new speed")
  @runtimeProperty("ModSettings.dependency", "cruiseEnabled")
  public let cruiseAfterOverride: ImmersiveDrivingAfterOverride = ImmersiveDrivingAfterOverride.ResumeSetSpeed;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Cruise Control")
  @runtimeProperty("ModSettings.category.order", "3")
  @runtimeProperty("ModSettings.displayName", "Brake cancels cruise control")
  @runtimeProperty("ModSettings.description", "When off, braking only pauses cruise control until you let go of the brake.")
  @runtimeProperty("ModSettings.dependency", "cruiseEnabled")
  public let cruiseCancelOnBrake: Bool = true;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Cruise Control")
  @runtimeProperty("ModSettings.category.order", "3")
  @runtimeProperty("ModSettings.displayName", "Handbrake cancels cruise control")
  @runtimeProperty("ModSettings.dependency", "cruiseEnabled")
  public let cruiseCancelOnHandbrake: Bool = true;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Cruise Control")
  @runtimeProperty("ModSettings.category.order", "3")
  @runtimeProperty("ModSettings.displayName", "Crashes cancel cruise control")
  @runtimeProperty("ModSettings.description", "Switch cruise control off after a sudden impact.")
  @runtimeProperty("ModSettings.dependency", "cruiseEnabled")
  public let cruiseCancelOnCollision: Bool = true;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Cruise Control")
  @runtimeProperty("ModSettings.category.order", "3")
  @runtimeProperty("ModSettings.displayName", "Brake to hold speed")
  @runtimeProperty("ModSettings.description", "Brake lightly downhill or after lowering the set speed. When off, the car only coasts.")
  @runtimeProperty("ModSettings.dependency", "cruiseEnabled")
  public let cruiseUseBrakes: Bool = true;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Cruise Control")
  @runtimeProperty("ModSettings.category.order", "3")
  @runtimeProperty("ModSettings.displayName", "Maximum cruise braking (%)")
  @runtimeProperty("ModSettings.min", "5")
  @runtimeProperty("ModSettings.max", "100")
  @runtimeProperty("ModSettings.step", "5")
  @runtimeProperty("ModSettings.dependency", "cruiseUseBrakes")
  public let cruiseMaxBrakePct: Int32 = 30;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Cruise Control")
  @runtimeProperty("ModSettings.category.order", "3")
  @runtimeProperty("ModSettings.displayName", "Maximum cruise throttle (%)")
  @runtimeProperty("ModSettings.description", "Lower values make cruise control climb hills and reach a higher set speed more gently.")
  @runtimeProperty("ModSettings.min", "10")
  @runtimeProperty("ModSettings.max", "100")
  @runtimeProperty("ModSettings.step", "5")
  @runtimeProperty("ModSettings.dependency", "cruiseEnabled")
  public let cruiseMaxThrottlePct: Int32 = 100;

  // ---------------------------------------------------------------------------------------------------------------------
  // Speed limiter

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Speed Limiter")
  @runtimeProperty("ModSettings.category.order", "4")
  @runtimeProperty("ModSettings.displayName", "Enable speed limiter")
  @runtimeProperty("ModSettings.description", "Keep the car from going faster than a set limit. Drive as usual, the throttle eases off at the limit. Limits are whole steps of 5 in the selected speed units. Switching the speed limiter on switches cruise control off, and the other way around.")
  public let limiterEnabled: Bool = true;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Speed Limiter")
  @runtimeProperty("ModSettings.category.order", "4")
  @runtimeProperty("ModSettings.displayName", "Default limit")
  @runtimeProperty("ModSettings.description", "The limit in the selected speed units when the speed limiter key is pressed below the minimum speed, for example parked, before any limit was set since loading the game.")
  @runtimeProperty("ModSettings.min", "10")
  @runtimeProperty("ModSettings.max", "300")
  @runtimeProperty("ModSettings.step", "5")
  @runtimeProperty("ModSettings.dependency", "limiterEnabled")
  public let limiterDefaultLimit: Int32 = 60;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Speed Limiter")
  @runtimeProperty("ModSettings.category.order", "4")
  @runtimeProperty("ModSettings.displayName", "Stay on after leaving the car")
  @runtimeProperty("ModSettings.description", "The speed limiter stays on when you get out, and limits the next car you drive. When off, leaving the driver seat switches it off. Loading a save always starts with it off.")
  @runtimeProperty("ModSettings.dependency", "limiterEnabled")
  public let limiterKeepOnExit: Bool = true;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Speed Limiter")
  @runtimeProperty("ModSettings.category.order", "4")
  @runtimeProperty("ModSettings.displayName", "Sport key passes the limit")
  @runtimeProperty("ModSettings.description", "Holding the Sport Mode key lifts the limit, for example to overtake. When you let go, the car slows back down to the limit. Sport mode switched on with a tap does not lift it.")
  @runtimeProperty("ModSettings.dependency", "limiterEnabled")
  public let limiterKickdown: Bool = true;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Speed Limiter")
  @runtimeProperty("ModSettings.category.order", "4")
  @runtimeProperty("ModSettings.displayName", "Brake to stay at the limit")
  @runtimeProperty("ModSettings.description", "Brake lightly downhill, after lowering the limit, or after passing it with the Sport key. When off, the car only coasts.")
  @runtimeProperty("ModSettings.dependency", "limiterEnabled")
  public let limiterUseBrakes: Bool = true;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Speed Limiter")
  @runtimeProperty("ModSettings.category.order", "4")
  @runtimeProperty("ModSettings.displayName", "Maximum limiter braking (%)")
  @runtimeProperty("ModSettings.min", "5")
  @runtimeProperty("ModSettings.max", "100")
  @runtimeProperty("ModSettings.step", "5")
  @runtimeProperty("ModSettings.dependency", "limiterUseBrakes")
  public let limiterMaxBrakePct: Int32 = 30;

  // ---------------------------------------------------------------------------------------------------------------------
  // Key bindings. The names are the overridableUI names in input/ImmersiveDriving.xml.

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Key Bindings")
  @runtimeProperty("ModSettings.category.order", "5")
  @runtimeProperty("ModSettings.displayName", "Sport Mode")
  @runtimeProperty("ModSettings.description", "Sport throttle, brake and steering levels, while held or switched on and off (see Sport Mode key).")
  public let immersiveDrivingSport: EInputKey = EInputKey.IK_LShift;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Key Bindings")
  @runtimeProperty("ModSettings.category.order", "5")
  @runtimeProperty("ModSettings.displayName", "Sport Mode key")
  @runtimeProperty("ModSettings.description", "Hold: Sport mode while the key is held. Toggle: press to switch Sport mode on, press again to switch it off. Tap or hold: a short tap toggles, holding the key gives Sport mode only while held, or default levels while held when Sport mode is on. A toggled mode also switches off when you leave the driver seat.")
  @runtimeProperty("ModSettings.displayValues.Hold", "Hold")
  @runtimeProperty("ModSettings.displayValues.Toggle", "Toggle")
  @runtimeProperty("ModSettings.displayValues.TapOrHold", "Tap or hold")
  public let sportKeyMode: ImmersiveDrivingKeyMode = ImmersiveDrivingKeyMode.TapOrHold;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Key Bindings")
  @runtimeProperty("ModSettings.category.order", "5")
  @runtimeProperty("ModSettings.displayName", "Gentle Mode")
  @runtimeProperty("ModSettings.description", "Gentle throttle, brake and steering levels, while held or switched on and off (see Gentle Mode key).")
  public let immersiveDrivingGentle: EInputKey = EInputKey.IK_Alt;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Key Bindings")
  @runtimeProperty("ModSettings.category.order", "5")
  @runtimeProperty("ModSettings.displayName", "Gentle Mode key")
  @runtimeProperty("ModSettings.description", "Hold: Gentle mode while the key is held. Toggle: press to switch Gentle mode on, press again to switch it off. Tap or hold: a short tap toggles, holding the key gives Gentle mode only while held, or default levels while held when Gentle mode is on. A toggled mode also switches off when you leave the driver seat.")
  @runtimeProperty("ModSettings.displayValues.Hold", "Hold")
  @runtimeProperty("ModSettings.displayValues.Toggle", "Toggle")
  @runtimeProperty("ModSettings.displayValues.TapOrHold", "Tap or hold")
  public let gentleKeyMode: ImmersiveDrivingKeyMode = ImmersiveDrivingKeyMode.TapOrHold;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Key Bindings")
  @runtimeProperty("ModSettings.category.order", "5")
  @runtimeProperty("ModSettings.displayName", "Cruise control on / off")
  @runtimeProperty("ModSettings.description", "Switch cruise control on at the current speed (rounded to a step of 5), or off. Switches the speed limiter off.")
  public let immersiveDrivingCruiseToggle: EInputKey = EInputKey.IK_Mouse5;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Key Bindings")
  @runtimeProperty("ModSettings.category.order", "5")
  @runtimeProperty("ModSettings.displayName", "Speed limiter on / off")
  @runtimeProperty("ModSettings.description", "Switch the speed limiter on at the current speed rounded up to a step of 5, or off. Below the minimum speed it uses the last limit, or the default limit. Switches cruise control off.")
  public let immersiveDrivingLimiterToggle: EInputKey = EInputKey.IK_Mouse4;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Key Bindings")
  @runtimeProperty("ModSettings.category.order", "5")
  @runtimeProperty("ModSettings.displayName", "Set speed up")
  @runtimeProperty("ModSettings.description", "Raise the cruise speed or the speed limit by 5, whichever is on. With both off, see Set speed keys.")
  public let immersiveDrivingCruiseFaster: EInputKey = EInputKey.IK_PageUp;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Key Bindings")
  @runtimeProperty("ModSettings.category.order", "5")
  @runtimeProperty("ModSettings.displayName", "Set speed down")
  @runtimeProperty("ModSettings.description", "Lower the cruise speed or the speed limit by 5, whichever is on. With both off, see Set speed keys.")
  public let immersiveDrivingCruiseSlower: EInputKey = EInputKey.IK_PageDown;

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Key Bindings")
  @runtimeProperty("ModSettings.category.order", "5")
  @runtimeProperty("ModSettings.displayName", "Set speed keys")
  @runtimeProperty("ModSettings.description", "What set speed up and down do while cruise control and the speed limiter are both off. Start cruise control: up switches it back on at the last cruise speed, down at the current speed. Start speed limiter: up switches it back on at the last limit, down at the current speed rounded up. Do nothing: they only change a speed that is already on.")
  @runtimeProperty("ModSettings.displayValues.StartCruise", "Start cruise control")
  @runtimeProperty("ModSettings.displayValues.StartLimiter", "Start speed limiter")
  @runtimeProperty("ModSettings.displayValues.Nothing", "Do nothing")
  public let setSpeedKeys: ImmersiveDrivingSetSpeedKeys = ImmersiveDrivingSetSpeedKeys.StartCruise;

  // ---------------------------------------------------------------------------------------------------------------------
  // Advanced

  @runtimeProperty("ModSettings.mod", "Drive Modes and Cruise Control")
  @runtimeProperty("ModSettings.category", "Advanced")
  @runtimeProperty("ModSettings.category.order", "6")
  @runtimeProperty("ModSettings.displayName", "Debug logging")
  @runtimeProperty("ModSettings.description", "Write driving values to red4ext/logs/ImmersiveDriving-*.log once per second, plus the mod's key presses and cruise control and speed limiter speed changes.")
  public let debugLogging: Bool = false;

  public func OnModSettingsChange() -> Void {
    this.Push();
    if IsDefined(this.system) {
      this.system.OnSettingsChanged();
    }
  }

  // Sends every native setting to the plugin.
  public func Push() -> Void {
    ImmersiveDriving_SetBool(n"enabled", this.enabled);
    ImmersiveDriving_SetBool(n"applyToCars", this.applyToCars);
    ImmersiveDriving_SetBool(n"applyToBikes", this.applyToBikes);
    ImmersiveDriving_SetBool(n"applyToGamepad", this.applyToGamepad);
    ImmersiveDriving_SetBool(n"debugLogging", this.debugLogging);

    ImmersiveDriving_SetInt(n"throttleNormalPct", this.throttleNormalPct);
    ImmersiveDriving_SetInt(n"throttleSportPct", this.throttleSportPct);
    ImmersiveDriving_SetInt(n"throttleGentlePct", this.throttleGentlePct);
    ImmersiveDriving_SetInt(n"brakeNormalPct", this.brakeNormalPct);
    ImmersiveDriving_SetInt(n"brakeSportPct", this.brakeSportPct);
    ImmersiveDriving_SetInt(n"brakeGentlePct", this.brakeGentlePct);
    ImmersiveDriving_SetInt(n"steerNormalPct", this.steerNormalPct);
    ImmersiveDriving_SetInt(n"steerSportPct", this.steerSportPct);
    ImmersiveDriving_SetInt(n"steerGentlePct", this.steerGentlePct);

    ImmersiveDriving_SetBool(n"steeringSmoothing", this.steeringSmoothing);
    ImmersiveDriving_SetFloat(n"steerRiseSec", this.steerRiseSec);
    ImmersiveDriving_SetBool(n"speedSensitiveSteering", this.speedSensitiveSteering);
    ImmersiveDriving_SetFloat(n"fullSteerBelowKmh", this.fullSteerBelowKmh);
    ImmersiveDriving_SetFloat(n"highSpeedKmh", this.highSpeedKmh);
    ImmersiveDriving_SetInt(n"highSpeedSteerPct", this.highSpeedSteerPct);

    ImmersiveDriving_SetBool(n"cruiseEnabled", this.cruiseEnabled);
    ImmersiveDriving_SetFloat(n"cruiseMinKmh", this.cruiseMinKmh);
    ImmersiveDriving_SetFloat(n"cruiseChangeRateKmhps", this.cruiseChangeRateKmhps);
    ImmersiveDriving_SetInt(n"cruiseResponsivenessPct", this.cruiseResponsivenessPct);
    ImmersiveDriving_SetInt(n"cruiseAfterOverride", EnumInt(this.cruiseAfterOverride));
    ImmersiveDriving_SetBool(n"cruiseCancelOnBrake", this.cruiseCancelOnBrake);
    ImmersiveDriving_SetBool(n"cruiseCancelOnHandbrake", this.cruiseCancelOnHandbrake);
    ImmersiveDriving_SetBool(n"cruiseCancelOnCollision", this.cruiseCancelOnCollision);
    ImmersiveDriving_SetBool(n"cruiseUseBrakes", this.cruiseUseBrakes);
    ImmersiveDriving_SetInt(n"cruiseMaxBrakePct", this.cruiseMaxBrakePct);
    ImmersiveDriving_SetInt(n"cruiseMaxThrottlePct", this.cruiseMaxThrottlePct);

    ImmersiveDriving_SetBool(n"limiterEnabled", this.limiterEnabled);
    ImmersiveDriving_SetBool(n"limiterKeepOnExit", this.limiterKeepOnExit);
    ImmersiveDriving_SetBool(n"limiterKickdown", this.limiterKickdown);
    ImmersiveDriving_SetBool(n"limiterUseBrakes", this.limiterUseBrakes);
    ImmersiveDriving_SetInt(n"limiterMaxBrakePct", this.limiterMaxBrakePct);
  }
}
