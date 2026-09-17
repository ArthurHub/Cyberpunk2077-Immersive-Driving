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
     * True in the vehicle's AutoDrive mode. Stays false during the patch 2.3 AutoDrive, which the scripts check through AutoDriveSystem.
     */
    bool isAutoDriveModeEnabled(RED4ext::ISerializable* vehicle, bool& enabled) noexcept;
}
