// Connects the game to the native plugin: tracks when the player drives, forwards the mod's keys, runs cruise control
// commands, and explains cruise control changes on screen.

public class ImmersiveDrivingSystem extends ScriptableSystem {

  private let settings: ref<ImmersiveDrivingSettings>;
  private let inputListener: ref<ImmersiveDrivingInputListener>;
  private let player: wref<PlayerPuppet>;
  private let vehicleStateCallback: ref<CallbackHandle>;
  private let vehicleState: gamePSMVehicle;
  private let vehicleID: EntityID;
  private let drivingAllowed: Bool;
  private let sportToggled: Bool;
  private let sportKeyHeld: Bool;
  private let gentleToggled: Bool;
  private let gentleKeyHeld: Bool;
  private let sportPressTime: Float;
  private let gentlePressTime: Float;
  private let pollScheduled: Bool;
  private let retryScheduled: Bool;
  private let refreshScheduled: Bool;
  private let waitingForDriverSeat: Bool;

  public static func Get(game: GameInstance) -> ref<ImmersiveDrivingSystem> {
    return GameInstance.GetScriptableSystemsContainer(game).Get(n"ImmersiveDrivingSystem") as ImmersiveDrivingSystem;
  }

  public func OnAttach() -> Void {
    // The plugin outlives game sessions, so start every session from a clean state.
    ImmersiveDriving_SetDrivingAllowed(false);
    this.ClearModes();
    ImmersiveDriving_ClearPlayerVehicle();

    this.settings = new ImmersiveDrivingSettings();
    this.settings.system = this;
    ImmersiveDrivingRegisterSettingsListener(this.settings);
    this.settings.Push();
  }

  public func OnDetach() -> Void {
    this.DetachFromPlayer();
    ImmersiveDrivingUnregisterSettingsListener(this.settings);
  }

  private func OnPlayerAttach(request: ref<PlayerAttachRequest>) -> Void {
    let player: ref<PlayerPuppet> = request.owner as PlayerPuppet;
    if IsDefined(player) {
      this.AttachToPlayer(player);
    }
  }

  private func OnPlayerDetach(request: ref<PlayerDetachRequest>) -> Void {
    this.DetachFromPlayer();
  }

  // -------------------------------------------------------------------------------------------------------------------
  // Player and vehicle tracking

  private func AttachToPlayer(player: ref<PlayerPuppet>) -> Void {
    this.DetachFromPlayer();
    this.player = player;

    this.inputListener = ImmersiveDrivingInputListener.Create(this);
    player.RegisterInputListener(this.inputListener, n"ImmersiveDriving_CruiseToggle");
    player.RegisterInputListener(this.inputListener, n"ImmersiveDriving_CruiseFaster");
    player.RegisterInputListener(this.inputListener, n"ImmersiveDriving_CruiseSlower");
    player.RegisterInputListener(this.inputListener, n"ImmersiveDriving_Gentle");
    player.RegisterInputListener(this.inputListener, n"ImmersiveDriving_Sport");
    player.RegisterInputListener(this.inputListener, n"__DEVICE_CHANGED__");
    ImmersiveDriving_SetUsingKeyboard(player.PlayerLastUsedKBM());

    this.RegisterVehicleStateListener();
  }

  private func DetachFromPlayer() -> Void {
    let player: ref<PlayerPuppet> = this.player;
    if IsDefined(player) {
      if IsDefined(this.inputListener) {
        player.UnregisterInputListener(this.inputListener);
      }
      let blackboard: ref<IBlackboard> = player.GetPlayerStateMachineBlackboard();
      if IsDefined(blackboard) && IsDefined(this.vehicleStateCallback) {
        blackboard.UnregisterListenerInt(GetAllBlackboardDefs().PlayerStateMachine.Vehicle, this.vehicleStateCallback);
      }
    }

    this.inputListener = null;
    this.vehicleStateCallback = null;
    this.player = null;
    this.vehicleState = gamePSMVehicle.Default;
    this.LeaveVehicle();
  }

  private func RegisterVehicleStateListener() -> Void {
    let player: ref<PlayerPuppet> = this.player;
    if !IsDefined(player) || IsDefined(this.vehicleStateCallback) {
      return;
    }

    let blackboard: ref<IBlackboard> = player.GetPlayerStateMachineBlackboard();
    if IsDefined(blackboard) {
      this.vehicleStateCallback = blackboard.RegisterListenerInt(GetAllBlackboardDefs().PlayerStateMachine.Vehicle, this, n"OnVehicleStateChanged", true);
    } else {
      // The state machine blackboard can appear a moment after the player attaches.
      this.Schedule(ImmersiveDrivingCallbackAction.RegisterListeners, 0.5);
    }
  }

  protected cb func OnVehicleStateChanged(value: Int32) -> Bool {
    this.vehicleState = IntEnum<gamePSMVehicle>(value);
    if IsDefined(this.settings) && this.settings.debugLogging {
      ImmersiveDriving_Log("Vehicle state " + EnumValueToString("gamePSMVehicle", Cast<Int64>(value)));
    }
    this.RefreshVehicle();
  }

  private func RefreshVehicle() -> Void {
    let player: ref<PlayerPuppet> = this.player;
    if !IsDefined(player) || Equals(this.vehicleState, gamePSMVehicle.Default) {
      this.waitingForDriverSeat = false;
      this.LeaveVehicle();
      return;
    }

    let game: GameInstance = player.GetGame();
    let vehicle: wref<VehicleObject>;
    if !VehicleComponent.GetVehicle(game, player.GetEntityID(), vehicle) || !VehicleComponent.IsDriver(game, player) {
      this.LeaveVehicle();
      // Seat switches (Auto Drive Enhanced) enter the driving state before the game has the player in the driver seat,
      // and the state does not change again, so keep checking while the state says the player drives.
      let driving: Bool = Equals(this.vehicleState, gamePSMVehicle.Driving) || Equals(this.vehicleState, gamePSMVehicle.DriverCombat);
      if driving {
        if !this.waitingForDriverSeat && this.settings.debugLogging {
          ImmersiveDriving_Log("Driving state without the driver seat, checking again");
        }
        this.Schedule(ImmersiveDrivingCallbackAction.RefreshVehicle, 0.25);
      }
      this.waitingForDriverSeat = driving;
      return;
    }
    this.waitingForDriverSeat = false;

    if !Equals(vehicle.GetEntityID(), this.vehicleID) {
      this.vehicleID = vehicle.GetEntityID();
      ImmersiveDriving_SetPlayerVehicle(vehicle, ImmersiveDrivingSystem.GetVehicleKind(vehicle));
    }

    this.RefreshDrivingAllowed();
    this.Schedule(ImmersiveDrivingCallbackAction.Poll, 0.1);
  }

  private func LeaveVehicle() -> Void {
    if !EntityID.IsDefined(this.vehicleID) {
      return;
    }

    let noVehicle: EntityID;
    this.vehicleID = noVehicle;
    this.drivingAllowed = false;
    ImmersiveDriving_SetDrivingAllowed(false);
    this.ClearModes();
    ImmersiveDriving_ClearPlayerVehicle();
  }

  // Whether the player really controls the car right now. Checked on vehicle state changes and while polling.
  private func RefreshDrivingAllowed() -> Void {
    let allowed: Bool = this.IsDrivingAllowed();
    if NotEquals(allowed, this.drivingAllowed) {
      this.drivingAllowed = allowed;
      ImmersiveDriving_SetDrivingAllowed(allowed);
    }
  }

  private func IsDrivingAllowed() -> Bool {
    let player: ref<PlayerPuppet> = this.player;
    if !IsDefined(player) || !EntityID.IsDefined(this.vehicleID) {
      return false;
    }
    if !Equals(this.vehicleState, gamePSMVehicle.Driving) && !Equals(this.vehicleState, gamePSMVehicle.DriverCombat) {
      return false;
    }

    // Quests restrict driving through status effects on the player, which swap in the no-drive input contexts.
    if StatusEffectSystem.ObjectHasStatusEffectWithTag(player, n"NoDriving") || StatusEffectSystem.ObjectHasStatusEffectWithTag(player, n"VehicleOnlyForward") {
      return false;
    }

    // Remote control and vehicle quickhacks force their own inputs.
    let vehicle: ref<VehicleObject> = GameInstance.FindEntityByID(player.GetGame(), this.vehicleID) as VehicleObject;
    if !IsDefined(vehicle) {
      return false;
    }
    return !vehicle.IsVehicleRemoteControlled() && !vehicle.IsVehicleAccelerateQuickhackActive() && !vehicle.IsVehicleForceBrakesQuickhackActive();
  }

  private static func GetVehicleKind(vehicle: ref<VehicleObject>) -> Int32 {
    if IsDefined(vehicle as BikeObject) {
      return 2;
    }
    if IsDefined(vehicle as CarObject) {
      return 1;
    }
    return 3;
  }

  // -------------------------------------------------------------------------------------------------------------------
  // Input

  public func OnInputAction(action: ListenerAction) -> Void {
    let name: CName = ListenerAction.GetName(action);
    if this.settings.debugLogging && NotEquals(name, n"__DEVICE_CHANGED__") {
      ImmersiveDriving_Log("Input " + NameToString(name) + " " + EnumValueToString("gameinputActionType", Cast<Int64>(EnumInt(ListenerAction.GetType(action)))) + " value " + FloatToString(ListenerAction.GetValue(action)));
    }

    switch name {
      case n"__DEVICE_CHANGED__":
        if IsDefined(this.player) {
          ImmersiveDriving_SetUsingKeyboard(this.player.PlayerLastUsedKBM());
        }
        break;
      case n"ImmersiveDriving_CruiseToggle":
        if ListenerAction.IsButtonJustPressed(action) {
          this.ToggleCruise();
        }
        break;
      case n"ImmersiveDriving_CruiseFaster":
        if ListenerAction.IsButtonJustPressed(action) {
          this.StepCruise(1);
        }
        break;
      case n"ImmersiveDriving_CruiseSlower":
        if ListenerAction.IsButtonJustPressed(action) {
          this.StepCruise(-1);
        }
        break;
      case n"ImmersiveDriving_Gentle":
        this.OnModeKey(action, false);
        break;
      case n"ImmersiveDriving_Sport":
        this.OnModeKey(action, true);
        break;
      default:
        break;
    }
  }

  // -------------------------------------------------------------------------------------------------------------------
  // Drive modes

  // A mode key works while held, switches its mode on and off, or both: a short tap toggles and a longer press holds
  // (Key Bindings). Switching one mode on with a toggle switches the other toggled mode off. A held key overrides a
  // toggled mode only while held.
  private func OnModeKey(action: ListenerAction, sport: Bool) -> Void {
    let pressed: Bool = ListenerAction.IsButtonJustPressed(action);
    if !pressed && !ListenerAction.IsButtonJustReleased(action) {
      return;
    }

    let mode: ImmersiveDrivingKeyMode = this.GetKeyMode(sport);
    let now: Float = EngineTime.ToFloat(GameInstance.GetSimTime(this.GetGameInstance()));
    let toggle: Bool = pressed && Equals(mode, ImmersiveDrivingKeyMode.Toggle);
    if sport {
      this.sportKeyHeld = pressed;
      if pressed {
        this.sportPressTime = now;
      } else {
        toggle = Equals(mode, ImmersiveDrivingKeyMode.TapOrHold) && now - this.sportPressTime < 0.3;
      }
    } else {
      this.gentleKeyHeld = pressed;
      if pressed {
        this.gentlePressTime = now;
      } else {
        toggle = Equals(mode, ImmersiveDrivingKeyMode.TapOrHold) && now - this.gentlePressTime < 0.3;
      }
    }

    if toggle {
      let active: Bool;
      if sport {
        this.sportToggled = !this.sportToggled;
        active = this.sportToggled;
      } else {
        this.gentleToggled = !this.gentleToggled;
        active = this.gentleToggled;
      }
      if active {
        if sport {
          this.gentleToggled = false;
        } else {
          this.sportToggled = false;
        }
      }
      this.Notify((sport ? "Sport" : "Gentle") + (active ? " mode on" : " mode off"));
      this.PlayClick();
    }
    this.PushModes();
  }

  private func GetKeyMode(sport: Bool) -> ImmersiveDrivingKeyMode {
    return sport ? this.settings.sportKeyMode : this.settings.gentleKeyMode;
  }

  // Sends the active modes to the plugin, with whether each key is physically down (only a held key can be a vanilla
  // lean key). A held key gives its mode while held and overrides the other toggled mode. Both modes are only active
  // together when both keys are held, and then Sport wins.
  private func PushModes() -> Void {
    let sportInverted: Bool = this.IsHoldInverted(true);
    let gentleInverted: Bool = this.IsHoldInverted(false);
    let sportHold: Bool = this.sportKeyHeld && NotEquals(this.GetKeyMode(true), ImmersiveDrivingKeyMode.Toggle) && !sportInverted;
    let gentleHold: Bool = this.gentleKeyHeld && NotEquals(this.GetKeyMode(false), ImmersiveDrivingKeyMode.Toggle) && !gentleInverted;
    ImmersiveDriving_SetSport(sportHold || (this.sportToggled && !gentleHold && !sportInverted), this.sportKeyHeld);
    ImmersiveDriving_SetGentle(gentleHold || (this.gentleToggled && !sportHold && !gentleInverted), this.gentleKeyHeld);
  }

  // Holding a Tap or hold key while its mode is toggled on does the opposite: the mode is off until the key is released.
  private func IsHoldInverted(sport: Bool) -> Bool {
    if sport {
      return this.sportKeyHeld && this.sportToggled && Equals(this.GetKeyMode(true), ImmersiveDrivingKeyMode.TapOrHold);
    }
    return this.gentleKeyHeld && this.gentleToggled && Equals(this.GetKeyMode(false), ImmersiveDrivingKeyMode.TapOrHold);
  }

  private func ClearModes() -> Void {
    this.sportToggled = false;
    this.sportKeyHeld = false;
    this.gentleToggled = false;
    this.gentleKeyHeld = false;
    ImmersiveDriving_SetSport(false, false);
    ImmersiveDriving_SetGentle(false, false);
  }

  // A mode that a toggle left on switches off when that key is changed to hold, which would never release it.
  public func OnSettingsChanged() -> Void {
    if Equals(this.GetKeyMode(true), ImmersiveDrivingKeyMode.Hold) {
      this.sportToggled = false;
    }
    if Equals(this.GetKeyMode(false), ImmersiveDrivingKeyMode.Hold) {
      this.gentleToggled = false;
    }
    this.PushModes();
  }

  // -------------------------------------------------------------------------------------------------------------------
  // Cruise control

  private func ToggleCruise() -> Void {
    if ImmersiveDriving_CancelCruise() {
      this.Notify("Cruise control off");
      this.PlayClick();
      return;
    }
    this.EngageCruise(ImmersiveDriving_GetSpeed());
  }

  private func StepCruise(direction: Int32) -> Void {
    if !ImmersiveDriving_IsCruiseActive() {
      let lastTarget: Float = ImmersiveDriving_GetLastCruiseTarget();
      this.EngageCruise(direction > 0 && lastTarget > 0.0 ? lastTarget : ImmersiveDriving_GetSpeed());
      return;
    }

    let game: GameInstance = this.GetGameInstance();
    let target: Float = this.SnapSpeed(ImmersiveDrivingSpeed.Step(game, this.settings.speedUnit, ImmersiveDriving_GetCruiseTarget(), direction));
    if ImmersiveDriving_SetCruiseTarget(target) {
      this.Notify("Cruise control " + this.FormatSpeed(ImmersiveDriving_GetCruiseTarget()));
    }
  }

  private func EngageCruise(targetSpeed: Float) -> Void {
    switch ImmersiveDriving_EngageCruise(this.SnapSpeed(targetSpeed)) {
      case 0:
        this.Notify("Cruise control " + this.FormatSpeed(ImmersiveDriving_GetCruiseTarget()));
        this.PlayClick();
        break;
      case 1:
        this.Notify("Cruise control is disabled in Mod Settings");
        break;
      case 3:
        this.Notify("Too slow for cruise control (minimum " + this.FormatSpeed(this.settings.cruiseMinKmh / 3.6) + ")");
        break;
      case 4:
        this.Notify("Drive Modes and Cruise Control is not ready, see red4ext/logs/ImmersiveDriving log");
        break;
      default:
        // Not driving (passenger, scene, AutoDrive, quickhack): stay silent.
        break;
    }
  }

  private func ExplainCruiseEvent(event: Int32) -> Void {
    switch event {
      case 1:
        // The plugin adopts the exact speed at which the pedal was released.
        ImmersiveDriving_SetCruiseTarget(this.SnapSpeed(ImmersiveDriving_GetCruiseTarget()));
        this.Notify("Cruise control " + this.FormatSpeed(ImmersiveDriving_GetCruiseTarget()));
        break;
      case 2:
      case 3:
        this.Notify("Cruise control off");
        this.PlayClick();
        break;
      case 4:
        this.Notify("Cruise control off (impact)");
        break;
      case 5:
        this.Notify("Cruise control off (too slow)");
        break;
      case 6:
        this.Notify("Cruise control off");
        break;
      default:
        break;
    }
  }

  private func SnapSpeed(speed: Float) -> Float {
    let snapped: Float = ImmersiveDrivingSpeed.Snap(this.GetGameInstance(), this.settings.speedUnit, speed, this.settings.cruiseMinKmh / 3.6);
    if this.settings.debugLogging {
      ImmersiveDriving_Log("Cruise speed " + FloatToString(speed * 3.6) + " km/h shows " + FloatToString(ImmersiveDrivingSpeed.ToDisplay(this.GetGameInstance(), this.settings.speedUnit, speed)) + ", snapped to " + FloatToString(snapped * 3.6) + " km/h which shows " + FloatToString(ImmersiveDrivingSpeed.ToDisplay(this.GetGameInstance(), this.settings.speedUnit, snapped)));
    }
    return snapped;
  }

  private func FormatSpeed(speed: Float) -> String {
    return ImmersiveDrivingSpeed.Format(this.GetGameInstance(), this.settings.speedUnit, speed);
  }

  // -------------------------------------------------------------------------------------------------------------------
  // Polling, messages and sounds

  private func Schedule(action: ImmersiveDrivingCallbackAction, delay: Float) -> Void {
    switch action {
      case ImmersiveDrivingCallbackAction.Poll:
        if this.pollScheduled {
          return;
        }
        this.pollScheduled = true;
        break;
      case ImmersiveDrivingCallbackAction.RefreshVehicle:
        if this.refreshScheduled {
          return;
        }
        this.refreshScheduled = true;
        break;
      default:
        if this.retryScheduled {
          return;
        }
        this.retryScheduled = true;
        break;
    }

    let callback: ref<ImmersiveDrivingCallback> = new ImmersiveDrivingCallback();
    callback.system = this;
    callback.action = action;
    GameInstance.GetDelaySystem(this.GetGameInstance()).DelayCallback(callback, delay, false);
  }

  public func OnCallback(action: ImmersiveDrivingCallbackAction) -> Void {
    if Equals(action, ImmersiveDrivingCallbackAction.RegisterListeners) {
      this.retryScheduled = false;
      this.RegisterVehicleStateListener();
      return;
    }
    if Equals(action, ImmersiveDrivingCallbackAction.RefreshVehicle) {
      this.refreshScheduled = false;
      this.RefreshVehicle();
      return;
    }

    this.pollScheduled = false;
    if !EntityID.IsDefined(this.vehicleID) {
      return;
    }

    this.RefreshDrivingAllowed();

    let remaining: Int32 = 16;
    let event: Int32 = ImmersiveDriving_PopCruiseEvent();
    while event != 0 && remaining > 0 {
      this.ExplainCruiseEvent(event);
      event = ImmersiveDriving_PopCruiseEvent();
      remaining -= 1;
    }

    this.Schedule(ImmersiveDrivingCallbackAction.Poll, 0.1);
  }

  private func Notify(message: String) -> Void {
    if !this.settings.showMessages {
      return;
    }

    let screenMessage: SimpleScreenMessage;
    screenMessage.isShown = true;
    screenMessage.duration = 2.0;
    screenMessage.message = message;
    GameInstance.GetBlackboardSystem(this.GetGameInstance()).Get(GetAllBlackboardDefs().UI_Notifications).SetVariant(GetAllBlackboardDefs().UI_Notifications.OnscreenMessage, ToVariant(screenMessage), true);
  }

  private func PlayClick() -> Void {
    if this.settings.playSounds {
      GameInstance.GetAudioSystem(this.GetGameInstance()).Play(n"ui_menu_onpress");
    }
  }
}

enum ImmersiveDrivingCallbackAction {
  Poll = 0,
  RegisterListeners = 1,
  RefreshVehicle = 2
}

public class ImmersiveDrivingCallback extends DelayCallback {

  public let system: wref<ImmersiveDrivingSystem>;
  public let action: ImmersiveDrivingCallbackAction;

  public func Call() -> Void {
    if IsDefined(this.system) {
      this.system.OnCallback(this.action);
    }
  }
}

public class ImmersiveDrivingInputListener extends IScriptable {

  private let system: wref<ImmersiveDrivingSystem>;

  public static func Create(system: ref<ImmersiveDrivingSystem>) -> ref<ImmersiveDrivingInputListener> {
    let listener: ref<ImmersiveDrivingInputListener> = new ImmersiveDrivingInputListener();
    listener.system = system;
    return listener;
  }

  protected cb func OnAction(action: ListenerAction, consumer: ListenerActionConsumer) -> Bool {
    if IsDefined(this.system) {
      this.system.OnInputAction(action);
    }
  }
}
