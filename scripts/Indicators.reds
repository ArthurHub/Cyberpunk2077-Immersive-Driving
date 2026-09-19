// On-screen indicators for the states that stay on after their message fades: cruise control, the speed limiter, and a
// mode left on by a toggle key. They are gameplay input hints, the same list the game fills while driving with change
// camera and draw weapon (ShowVehicleDriverInputHints), so the game handles placement, the key glyph, and hiding the
// hints with the rest of the HUD. Only changes are sent, the system can refresh as often as it likes.

public class ImmersiveDrivingIndicators extends IScriptable {

  // The hints are keyed by their action, so one field per hint holds what is shown right now ("" is nothing).
  private let cruiseLabel: String;
  private let limiterLabel: String;
  // Which key each hint shows, which depends on what the two keys do (Key Bindings).
  private let cruiseAction: CName;
  private let limiterAction: CName;
  private let sportShown: Bool;
  private let gentleShown: Bool;
  // Where the hints sit in the list, from Indicator placement (Mod Settings). Each hint sorts at this plus its own offset.
  private let basePriority: Int32;

  public func Refresh(game: GameInstance, basePriority: Int32, cruiseAction: CName, cruiseSpeed: String, limiterAction: CName, limiterSpeed: String, sport: Bool, gentle: Bool) -> Void {
    // A new placement only reaches a hint that is sent again, so everything on screen goes down and comes back below.
    if NotEquals(basePriority, this.basePriority) {
      this.Clear(game);
      this.basePriority = basePriority;
    }

    let cruise: String = StrLen(cruiseSpeed) > 0 ? "Cruise " + cruiseSpeed : "";
    let limiter: String = StrLen(limiterSpeed) > 0 ? "Limit " + limiterSpeed : "";

    // Everything that goes away is sent before anything that appears. Cruise control and the speed limiter can share
    // one key, and then both hints carry that key's action, so hiding one after showing the other would take the new
    // hint down with the old one. The key a hint shows also changes when the settings change.
    if StrLen(this.cruiseLabel) > 0 && (NotEquals(cruise, this.cruiseLabel) || NotEquals(cruiseAction, this.cruiseAction)) {
      this.Send(game, this.cruiseAction, "", 0);
      this.cruiseLabel = "";
    }
    if StrLen(this.limiterLabel) > 0 && (NotEquals(limiter, this.limiterLabel) || NotEquals(limiterAction, this.limiterAction)) {
      this.Send(game, this.limiterAction, "", 1);
      this.limiterLabel = "";
    }
    this.cruiseAction = cruiseAction;
    this.limiterAction = limiterAction;

    if NotEquals(cruise, this.cruiseLabel) {
      this.Send(game, this.cruiseAction, cruise, 0);
      this.cruiseLabel = cruise;
    }
    if NotEquals(limiter, this.limiterLabel) {
      this.Send(game, this.limiterAction, limiter, 1);
      this.limiterLabel = limiter;
    }
    if NotEquals(sport, this.sportShown) {
      this.Send(game, n"ImmersiveDriving_Sport", sport ? "Sport mode" : "", 2);
      this.sportShown = sport;
    }
    if NotEquals(gentle, this.gentleShown) {
      this.Send(game, n"ImmersiveDriving_Gentle", gentle ? "Gentle mode" : "", 3);
      this.gentleShown = gentle;
    }
  }

  public func Clear(game: GameInstance) -> Void {
    if StrLen(this.cruiseLabel) == 0 && StrLen(this.limiterLabel) == 0 && !this.sportShown && !this.gentleShown {
      return;
    }
    this.cruiseLabel = "";
    this.limiterLabel = "";
    this.sportShown = false;
    this.gentleShown = false;

    let event: ref<DeleteInputHintBySourceEvent> = new DeleteInputHintBySourceEvent();
    event.source = n"ImmersiveDriving";
    event.targetHintContainer = n"GameplayInputHelper";
    GameInstance.GetUISystem(game).QueueEvent(event);
  }

  // The game sends its exit vehicle hint only when the driving state changes, so a change to Hide the exit vehicle hint
  // reaches the car the player is in right now from here: the same hint the game sends (VehicleDriverContextEvents), or
  // nothing to take it down.
  public func SendExitHint(game: GameInstance, show: Bool, bike: Bool) -> Void {
    let data: InputHintData;
    data.action = n"Exit";
    data.source = n"VehicleDriver";
    if show {
      data.localizedLabel = bike ? "LocKey#53066" : "LocKey#36196";
      data.holdIndicationType = inkInputHintHoldIndicationType.FromInputConfig;
      data.enableHoldAnimation = true;
      data.sortingPriority = 127;
    }

    let event: ref<UpdateInputHintEvent> = new UpdateInputHintEvent();
    event.data = data;
    event.show = show;
    event.targetHintContainer = n"GameplayInputHelper";
    GameInstance.GetUISystem(game).QueueEvent(event);
  }

  // An empty label removes the hint. The label is plain text, not a LocKey, because the mod's messages are not localized.
  // The offset keeps the indicators in the same order wherever the placement puts them.
  private func Send(game: GameInstance, action: CName, label: String, sortingOffset: Int32) -> Void {
    let data: InputHintData;
    data.action = action;
    data.source = n"ImmersiveDriving";
    data.localizedLabel = label;
    data.sortingPriority = this.basePriority + sortingOffset;

    let event: ref<UpdateInputHintEvent> = new UpdateInputHintEvent();
    event.data = data;
    event.show = StrLen(label) > 0;
    event.targetHintContainer = n"GameplayInputHelper";
    GameInstance.GetUISystem(game).QueueEvent(event);
  }
}

// The game's exit vehicle hint sorts last (127), where mods that want to come last put their hints too, so it takes a
// line of a crowded hint list. Hide the exit vehicle hint (General) drops it for the driver only: passengers, remote
// control and AutoDrive use the same function with another source and keep their hint.
@wrapMethod(InputContextTransitionEvents)
protected final const func ShowVehicleExitInputHint(stateContext: ref<StateContext>, scriptInterface: ref<StateGameScriptInterface>, source: CName) -> Void {
  let system: ref<ImmersiveDrivingSystem> = ImmersiveDrivingSystem.Get(scriptInterface.owner.GetGame());
  if Equals(source, n"VehicleDriver") && IsDefined(system) && system.IsHidingExitHint() {
    this.RemoveInputHint(scriptInterface, n"Exit", source);
    // The game's own per frame update calls this again for as long as the parameter disagrees with the blocked state.
    stateContext.SetPermanentBoolParameter(n"IsExitVehicleBlocked", this.IsExitVehicleBlocked(scriptInterface), true);
    return;
  }
  wrappedMethod(stateContext, scriptInterface, source);
}
