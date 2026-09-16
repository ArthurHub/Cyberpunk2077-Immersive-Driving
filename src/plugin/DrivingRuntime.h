#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>
#include <string_view>

#include <RED4ext/Handle.hpp>
#include <RED4ext/ISerializable.hpp>

#include "core/Config.h"
#include "core/DriveController.h"

namespace immersive_driving
{
    /**
     * Driving state shared by the vehicle hook (game thread) and the script natives.
     *
     * Every tick the game zeroes the vehicle's driving inputs, recomputes them from the input actions, updates the
     * camera inputs, and then lets the vehicle consume them. The hook runs right after the camera update, reads the
     * game's values for the player's vehicle, and replaces them with shaped ones before they are consumed.
     */
    class DrivingRuntime
    {
    public:
        static DrivingRuntime& get() noexcept;

        /**
         * Called after vehicle::BaseObject::UpdateVehicleCameraInput, for every vehicle.
         */
        void afterInputUpdate(RED4ext::ISerializable* vehicle) noexcept;

        void setHookInstalled(bool installed) noexcept;

        bool setBool(std::string_view name, bool value);
        bool setFloat(std::string_view name, float value);
        bool setInt(std::string_view name, int value);

        void setPlayerVehicle(const RED4ext::Handle<RED4ext::ISerializable>& vehicle, VehicleKind kind);
        void clearPlayerVehicle();
        void setDrivingAllowed(bool allowed);
        void setUsingKeyboard(bool usingKeyboard);
        /**
         * The mode and whether its key is held: a toggled mode stays active after the key is released.
         */
        void setGentle(bool active, bool keyHeld);
        void setSport(bool active, bool keyHeld);

        EngageResult engageCruise(float targetSpeed);
        bool setCruiseTarget(float targetSpeed);
        bool cancelCruise();
        bool isCruiseActive();
        float getCruiseTarget();
        float getLastCruiseTarget();

        EngageResult engageLimiter(float limit);
        bool setLimiterTarget(float limit);
        bool cancelLimiter();
        bool isLimiterActive();
        float getLimiterTarget();
        float getLastLimiterTarget();

        float getSpeed();
        CruiseEvent popCruiseEvent();

        bool isReady() const noexcept;
        std::string getStatus();

    private:
        void logTick(const DriveInputs& game, const TickResult& result);

        std::mutex _mutex;

        // Lock-free copies, used to skip the many vehicles that are not the player's.
        std::atomic<RED4ext::ISerializable*> _playerVehiclePtr{ nullptr };
        std::atomic<bool> _hookInstalled{ false };
        std::atomic<std::uint64_t> _playerTicks{ 0 };

        // Guarded by _mutex.
        Config _config;
        DriveController _controller;
        RED4ext::WeakHandle<RED4ext::ISerializable> _playerVehicle;
        VehicleKind _vehicleKind = VehicleKind::Unknown;

        bool _drivingAllowed = false;
        bool _usingKeyboard = true;
        bool _gentle = false;
        bool _sport = false;
        bool _gentleKeyHeld = false;
        bool _sportKeyHeld = false;

        bool _speedAvailable = true;
        bool _autoDrive = false;
        std::chrono::steady_clock::time_point _lastTick{};
        std::chrono::steady_clock::time_point _lastTickLog{};
        DriveInputs _lastGame;
        TickResult _lastResult;
    };
}
