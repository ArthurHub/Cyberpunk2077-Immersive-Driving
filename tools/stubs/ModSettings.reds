// Compile-check stub of the Mod Settings API used by the mod (https://github.com/jackhumbert/mod_settings).
// Only used by tools/check-scripts.ps1, never shipped: the real class comes from the installed Mod Settings.

public native class ModSettings extends IScriptable {
  public native static func RegisterListenerToClass(self: ref<IScriptable>) -> Void;
  public native static func UnregisterListenerToClass(self: ref<IScriptable>) -> Void;
  public native static func RegisterListenerToModifications(self: ref<IScriptable>) -> Void;
  public native static func UnregisterListenerToModifications(self: ref<IScriptable>) -> Void;
}
