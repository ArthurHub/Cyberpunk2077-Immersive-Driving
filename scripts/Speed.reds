// Speed conversions between the real speed (m/s) used natively and the units shown to the player.

public abstract class ImmersiveDrivingSpeed {

  // Game setting Interface > Speedometer units: 0 metric, 1 imperial. Only the HUD speedometer follows it.
  public static func GameUsesKmh(game: GameInstance) -> Bool {
    let units: ref<ConfigVarListString> = GameInstance.GetSettingsSystem(game).GetVar(n"/interface", n"SpeedometerUnits") as ConfigVarListString;
    return IsDefined(units) && units.GetIndex() != 1;
  }

  public static func UnitLabel(game: GameInstance, unit: ImmersiveDrivingSpeedUnit) -> String {
    switch unit {
      case ImmersiveDrivingSpeedUnit.Kmh:
        return "km/h";
      case ImmersiveDrivingSpeedUnit.HudSpeedometer:
        return ImmersiveDrivingSpeed.GameUsesKmh(game) ? "km/h" : "mph";
      default:
        return "mph";
    }
  }

  // The number shown to the player for a real speed in m/s.
  public static func ToDisplay(game: GameInstance, unit: ImmersiveDrivingSpeedUnit, speed: Float) -> Float {
    let mps: Float = AbsF(speed);
    switch unit {
      case ImmersiveDrivingSpeedUnit.Kmh:
        return mps * 3.6;
      case ImmersiveDrivingSpeedUnit.Mph:
        return mps * 2.2369363;
      case ImmersiveDrivingSpeedUnit.HudSpeedometer:
        return ImmersiveDrivingSpeed.GameUsesKmh(game) ? ImmersiveDrivingSpeed.ToSpeedometer(game, mps) * 1.61 : ImmersiveDrivingSpeed.ToSpeedometer(game, mps);
      default:
        return ImmersiveDrivingSpeed.ToSpeedometer(game, mps);
    }
  }

  // Inverse of ToDisplay. The speedometer curve has no closed-form inverse, so it is solved by bisection.
  public static func FromDisplay(game: GameInstance, unit: ImmersiveDrivingSpeedUnit, shown: Float) -> Float {
    switch unit {
      case ImmersiveDrivingSpeedUnit.Kmh:
        return shown / 3.6;
      case ImmersiveDrivingSpeedUnit.Mph:
        return shown / 2.2369363;
      default:
        break;
    }

    let low: Float = 0.0;
    let high: Float = 200.0;
    let i: Int32 = 0;
    while i < 30 {
      let middle: Float = (low + high) * 0.5;
      if ImmersiveDrivingSpeed.ToDisplay(game, unit, middle) < shown {
        low = middle;
      } else {
        high = middle;
      }
      i += 1;
    }
    return (low + high) * 0.5;
  }

  // Formats a real speed, for example "72 km/h". RoundMath like the speedometer, RoundF truncates (24.99 would show 24).
  public static func Format(game: GameInstance, unit: ImmersiveDrivingSpeedUnit, speed: Float) -> String {
    return IntToString(RoundMath(ImmersiveDrivingSpeed.ToDisplay(game, unit, speed))) + " " + ImmersiveDrivingSpeed.UnitLabel(game, unit);
  }

  // Cruise control speeds are whole steps of this size in the shown units.
  public static func StepSize() -> Float {
    return 5.0;
  }

  // The nearest whole step to a real speed, and never below the minimum speed.
  public static func Snap(game: GameInstance, unit: ImmersiveDrivingSpeedUnit, speed: Float, minSpeed: Float) -> Float {
    let size: Float = ImmersiveDrivingSpeed.StepSize();
    let shown: Float = Cast<Float>(RoundMath(ImmersiveDrivingSpeed.ToDisplay(game, unit, speed) / size)) * size;
    let lowest: Float = Cast<Float>(CeilF(ImmersiveDrivingSpeed.ToDisplay(game, unit, minSpeed) / size - 0.01)) * size;
    return ImmersiveDrivingSpeed.SpeedShowing(game, unit, MaxF(shown, lowest));
  }

  // A real speed that is shown as the given whole number. Bisection is exact for true units, but the speedometer curve
  // may not rise smoothly, so the result is checked and, if needed, the nearby speeds are searched for the closest one.
  public static func SpeedShowing(game: GameInstance, unit: ImmersiveDrivingSpeedUnit, shown: Float) -> Float {
    let speed: Float = ImmersiveDrivingSpeed.FromDisplay(game, unit, shown);
    if AbsF(ImmersiveDrivingSpeed.ToDisplay(game, unit, speed) - shown) < 0.05 {
      return speed;
    }

    let best: Float = speed;
    let bestError: Float = AbsF(ImmersiveDrivingSpeed.ToDisplay(game, unit, speed) - shown);
    let candidate: Float = MaxF(speed - 10.0, 0.0);
    let last: Float = speed + 10.0;
    while candidate <= last {
      let error: Float = AbsF(ImmersiveDrivingSpeed.ToDisplay(game, unit, candidate) - shown);
      if error < bestError {
        best = candidate;
        bestError = error;
      }
      candidate += 0.02;
    }
    return best;
  }

  // The next set speed one step up or down, snapped to whole steps in the shown units (for example 83 -> 85 -> 90).
  public static func Step(game: GameInstance, unit: ImmersiveDrivingSpeedUnit, speed: Float, direction: Int32) -> Float {
    let size: Float = ImmersiveDrivingSpeed.StepSize();
    let steps: Float = ImmersiveDrivingSpeed.ToDisplay(game, unit, speed) / size;
    let next: Float;
    if direction > 0 {
      next = Cast<Float>(FloorF(steps + 0.01) + 1) * size;
    } else {
      next = Cast<Float>(CeilF(steps - 0.01) - 1) * size;
    }
    return ImmersiveDrivingSpeed.SpeedShowing(game, unit, MaxF(next, 0.0));
  }

  // The game's speedometer number, before any km/h conversion. The car's own display shows it as is
  // (speedometerLogicController), the HUD speedometer multiplies it by 1.61 for metric (hudCarController). Both read
  // VehicleObject.GetCurrentSpeed, the same speed the plugin uses.
  private static func ToSpeedometer(game: GameInstance, mps: Float) -> Float {
    let multiplier: Float = GameInstance.GetStatsDataSystem(game).GetValueFromCurve(n"vehicle_ui", mps, n"speed_to_multiplier");
    if multiplier <= 0.0 {
      multiplier = 2.2369363;
    }
    return mps * multiplier;
  }
}
