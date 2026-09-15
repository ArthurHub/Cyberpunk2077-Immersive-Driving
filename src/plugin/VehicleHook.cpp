#include "VehicleHook.h"

#include <cstdint>

#include <RED4ext/Relocation.hpp>
#include <RED4ext/Scripting/Natives/vehicleBaseObject.hpp>

#include "DrivingRuntime.h"
#include "Log.h"

namespace immersive_driving::vehicle_hook
{
    namespace
    {
        // RED4ext address hash of vehicle::BaseObject::UpdateVehicleCameraInput (also hooked by Let There Be Flight).
        // Patch 2.31 lists it in bin/x64/cyberpunk2077_addresses.json. The game calls it right after computing the
        // driving inputs and right before the vehicle consumes them, which makes it the place to shape them.
        constexpr std::uint32_t UPDATE_VEHICLE_CAMERA_INPUT_HASH = 501486464u;

        using UpdateVehicleCameraInputFn = void (*)(RED4ext::vehicle::BaseObject* vehicle);

        UpdateVehicleCameraInputFn originalUpdate = nullptr;
        void* hookTarget = nullptr;

        void updateVehicleCameraInputDetour(RED4ext::vehicle::BaseObject* vehicle)
        {
            originalUpdate(vehicle);
            DrivingRuntime::get().afterInputUpdate(vehicle);
        }
    }

    bool attach(const RED4ext::v1::PluginHandle handle, const RED4ext::v1::Sdk* sdk)
    {
        if (!sdk || !sdk->hooking || !sdk->hooking->Attach) {
            logger::error("RED4ext hooking API is unavailable");
            return false;
        }

        hookTarget = reinterpret_cast<void*>(RED4ext::UniversalRelocBase::Resolve(UPDATE_VEHICLE_CAMERA_INPUT_HASH));
        if (!sdk->hooking->Attach(handle, hookTarget, reinterpret_cast<void*>(&updateVehicleCameraInputDetour), reinterpret_cast<void**>(&originalUpdate))) {
            logger::error("Could not hook vehicle::BaseObject::UpdateVehicleCameraInput");
            hookTarget = nullptr;
            return false;
        }

        DrivingRuntime::get().setHookInstalled(true);
        logger::info("Hooked vehicle::BaseObject::UpdateVehicleCameraInput");
        return true;
    }

    void detach(const RED4ext::v1::PluginHandle handle, const RED4ext::v1::Sdk* sdk)
    {
        if (!hookTarget || !sdk || !sdk->hooking || !sdk->hooking->Detach) {
            return;
        }

        DrivingRuntime::get().setHookInstalled(false);
        sdk->hooking->Detach(handle, hookTarget);
        hookTarget = nullptr;
    }
}
