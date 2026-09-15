#pragma once

namespace RED4ext
{
    struct ISerializable;
}

/**
 * Calls VehicleObject methods through RTTI. Each returns false when the game does not have the method.
 */
namespace immersive_driving::vehicle_functions
{
    /**
     * Signed forward speed in m/s (the value the speedometer uses).
     */
    bool getCurrentSpeed(RED4ext::ISerializable* vehicle, float& speed) noexcept;

    /**
     * True while the patch 2.3 AutoDrive is driving the car.
     */
    bool isAutoDriveModeEnabled(RED4ext::ISerializable* vehicle, bool& enabled) noexcept;
}
