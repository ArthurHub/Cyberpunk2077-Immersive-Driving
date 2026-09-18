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

  public func Refresh(game: GameInstance, cruiseAction: CName, cruiseSpeed: String, limiterAction: CName, limiterSpeed: String, sport: Bool, gentle: Bool) -> Void {
    let cruise: String = StrLen(cruiseSpeed) > 0 ? "Cruise " + cruiseSpeed : "";
    let limiter: String = StrLen(limiterSpeed) > 0 ? "Limit " + limiterSpeed : "";

    // Everything that goes away is sent before anything that appears. Cruise control and the speed limiter can share
    // one key, and then both hints carry that key's action, so hiding one after showing the other would take the new
    // hint down with the old one. The key a hint shows also changes when the settings change.
    if StrLen(this.cruiseLabel) > 0 && (NotEquals(cruise, this.cruiseLabel) || NotEquals(cruiseAction, this.cruiseAction)) {
      this.Send(game, this.cruiseAction, "", 10);
      this.cruiseLabel = "";
    }
    if StrLen(this.limiterLabel) > 0 && (NotEquals(limiter, this.limiterLabel) || NotEquals(limiterAction, this.limiterAction)) {
      this.Send(game, this.limiterAction, "", 11);
      this.limiterLabel = "";
    }
    this.cruiseAction = cruiseAction;
    this.limiterAction = limiterAction;

    if NotEquals(cruise, this.cruiseLabel) {
      this.Send(game, this.cruiseAction, cruise, 10);
      this.cruiseLabel = cruise;
    }
    if NotEquals(limiter, this.limiterLabel) {
      this.Send(game, this.limiterAction, limiter, 11);
      this.limiterLabel = limiter;
    }
    if NotEquals(sport, this.sportShown) {
      this.Send(game, n"ImmersiveDriving_Sport", sport ? "Sport mode" : "", 12);
      this.sportShown = sport;
    }
    if NotEquals(gentle, this.gentleShown) {
      this.Send(game, n"ImmersiveDriving_Gentle", gentle ? "Gentle mode" : "", 13);
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

  // An empty label removes the hint. The label is plain text, not a LocKey, because the mod's messages are not localized.
  private func Send(game: GameInstance, action: CName, label: String, sortingPriority: Int32) -> Void {
    let data: InputHintData;
    data.action = action;
    data.source = n"ImmersiveDriving";
    data.localizedLabel = label;
    // Vanilla driver hints (change camera, draw weapon) sort at 1 to 3, so these follow them.
    data.sortingPriority = sortingPriority;

    let event: ref<UpdateInputHintEvent> = new UpdateInputHintEvent();
    event.data = data;
    event.show = StrLen(label) > 0;
    event.targetHintContainer = n"GameplayInputHelper";
    GameInstance.GetUISystem(game).QueueEvent(event);
  }
}
