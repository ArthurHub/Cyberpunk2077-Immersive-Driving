#include <RED4ext/RED4ext.hpp>

#include <Version.h>

#include "Log.h"
#include "Natives.h"
#include "VehicleHook.h"

using namespace immersive_driving;

RED4EXT_C_EXPORT bool RED4EXT_CALL Main(RED4ext::v1::PluginHandle handle, RED4ext::v1::EMainReason reason, const RED4ext::v1::Sdk* sdk)
{
    switch (reason) {
    case RED4ext::v1::EMainReason::Load: {
        logger::init(handle, sdk);
        logger::info("Drive Modes and Cruise Control %s loading", IMMERSIVE_DRIVING_VERSION_STRING);

        natives::registerAll();

        // The scripts ship next to the DLL and are only compiled when the plugin actually loads.
        if (!sdk->scripts || !sdk->scripts->Add(handle, L"scripts")) {
            logger::error("Could not add the redscript folder to the compilation");
        }

        vehicle_hook::attach(handle, sdk);
        break;
    }
    case RED4ext::v1::EMainReason::Unload: {
        vehicle_hook::detach(handle, sdk);
        break;
    }
    }

    return true;
}

RED4EXT_C_EXPORT void RED4EXT_CALL Query(RED4ext::v1::PluginInfo* info)
{
    info->name = L"ImmersiveDriving";
    info->author = L"ArthurHub";
    info->version = RED4EXT_V1_SEMVER(IMMERSIVE_DRIVING_VERSION_MAJOR, IMMERSIVE_DRIVING_VERSION_MINOR, IMMERSIVE_DRIVING_VERSION_PATCH);
    info->runtime = RED4EXT_V1_RUNTIME_VERSION_2_31;
    // SDK 1.0.0 is compatible with 0.5.0, and RED4ext 1.29.x refuses plugins reporting anything newer.
    info->sdk = RED4EXT_V1_SDK_VERSION_1_0_0_COMPAT_0_5_0;
}

RED4EXT_C_EXPORT uint32_t RED4EXT_CALL Supports()
{
    // API v0 and v1 are compatible, reporting v0 keeps the plugin loadable by RED4ext 1.29.x as well.
    return RED4EXT_API_VERSION_1_COMPAT_0;
}
