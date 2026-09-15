// Mod Settings is what makes everything configurable in-game. Without it the scripts still compile and the mod runs with
// the default values from Settings.reds, instead of breaking script compilation for every installed mod.

// The settings object needs both registrations: the class listener only copies the accepted values into its fields, and
// OnModSettingsChange() is only called on modification listeners. Applying copies the fields first, then notifies.
@if(ModuleExists("ModSettingsModule"))
public func ImmersiveDrivingRegisterSettingsListener(listener: ref<IScriptable>) -> Void {
  ModSettings.RegisterListenerToClass(listener);
  ModSettings.RegisterListenerToModifications(listener);
}

@if(!ModuleExists("ModSettingsModule"))
public func ImmersiveDrivingRegisterSettingsListener(listener: ref<IScriptable>) -> Void {}

@if(ModuleExists("ModSettingsModule"))
public func ImmersiveDrivingUnregisterSettingsListener(listener: ref<IScriptable>) -> Void {
  ModSettings.UnregisterListenerToModifications(listener);
  ModSettings.UnregisterListenerToClass(listener);
}

@if(!ModuleExists("ModSettingsModule"))
public func ImmersiveDrivingUnregisterSettingsListener(listener: ref<IScriptable>) -> Void {}
