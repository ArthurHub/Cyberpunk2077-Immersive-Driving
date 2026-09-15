#pragma once

#include <RED4ext/Api/v1/PluginHandle.hpp>
#include <RED4ext/Api/v1/Sdk.hpp>

/**
 * Detour on vehicle::BaseObject::UpdateVehicleCameraInput, the per-tick update that turns the player's input
 * actions into the vehicle's throttle, brake and steering values.
 */
namespace immersive_driving::vehicle_hook
{
    bool attach(RED4ext::v1::PluginHandle handle, const RED4ext::v1::Sdk* sdk);
    void detach(RED4ext::v1::PluginHandle handle, const RED4ext::v1::Sdk* sdk);
}
