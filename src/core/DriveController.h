#pragma once

#include <array>
#include <cstddef>

#include "Config.h"

namespace immersive_driving
{
    enum class VehicleKind : int
    {
        Unknown = 0,
        Car = 1,
        Bike = 2,
        Unsupported = 3,
    };

    /**
     * Cruise changes the scripts did not ask for. They are queued so the HUD can explain them.
     */
    enum class CruiseEvent : int
    {
        None = 0,
        TargetAdopted = 1,
        CancelledByBrake = 2,
        CancelledByHandbrake = 3,
        CancelledByCollision = 4,
        CancelledTooSlow = 5,
        CancelledUnavailable = 6,
    };

    enum class EngageResult : int
    {
        Engaged = 0,
        Disabled = 1,
        NotDriving = 2,
        TooSlow = 3,
        NotReady = 4,
    };

    /**
     * Driving inputs as stored on the vehicle: throttle and brake are 0..1, steer and lean are -1..1 (positive = right,
     * forward).
     */
    struct DriveInputs
    {
        float accelerate = 0.0f;
        float decelerate = 0.0f;
        float steer = 0.0f;
        float lean = 0.0f;
    };

    struct TickContext
    {
        float deltaTime = 0.0f;
        // Signed forward speed in m/s, negative while reversing.
        float speed = 0.0f;
        // What the game computed from the player's input this tick.
        DriveInputs game;
        // Handbrake action value reported by the scripts.
        float handbrake = 0.0f;
        bool usingKeyboard = true;
        // The Sport and Gentle modes select the throttle, brake and steering levels. Sport wins when both are active.
        bool sport = false;
        bool gentle = false;
        // A mode key is physically down. A toggled mode stays active without it, and only a key that is really held can
        // be a vanilla lean key, so the lean pairing fix follows this and not the modes.
        bool modeKeyHeld = false;
        // The player drives and nothing else (scene, autodrive, ...) owns the car.
        bool drivingAllowed = false;
        VehicleKind vehicleKind = VehicleKind::Unknown;
    };

    struct TickResult
    {
        DriveInputs output;
        bool write = false;
    };

    /**
     * Turns the game's raw driving inputs into shaped ones: normal, Sport and Gentle levels for throttle, brake and steering,
     * smooth and speed-sensitive steering, and cruise control. Speeds are in m/s.
     */
    class DriveController
    {
    public:
        TickResult tick(const Config& config, const TickContext& context);

        EngageResult engageCruise(const Config& config, float targetSpeed);
        bool setCruiseTarget(const Config& config, float targetSpeed);

        /**
         * Returns true when cruise control was active. Only reasons other than the driver's own request are queued.
         */
        bool cancelCruise(CruiseEvent reason = CruiseEvent::None);

        [[nodiscard]] bool isCruiseActive() const noexcept;
        [[nodiscard]] float getCruiseTarget() const noexcept;
        [[nodiscard]] float getLastCruiseTarget() const noexcept;
        [[nodiscard]] float getSpeed() const noexcept;
        [[nodiscard]] bool isAvailable() const noexcept;

        CruiseEvent popEvent() noexcept;

        /**
         * Forgets everything tied to the current vehicle.
         */
        void reset() noexcept;

    private:
        struct SpeedSample
        {
            float time = 0.0f;
            float speed = 0.0f;
        };

        struct CruiseState
        {
            bool active = false;
            bool overriding = false;
            bool braking = false;
            float target = 0.0f;
            float rampTarget = 0.0f;
            float integral = 0.0f;
            float throttle = 0.0f;
            float brake = 0.0f;
            float slowTime = 0.0f;
        };

        float shapeSteering(const Config& config, const TickContext& context, float input, float deltaTime, bool shape);
        void updateCruise(const Config& config, const TickContext& context, float deltaTime, DriveInputs& output);

        void recordSpeed(float speed) noexcept;
        [[nodiscard]] bool detectCollision() const noexcept;
        void pushEvent(CruiseEvent event) noexcept;

        float _throttle = 0.0f;
        float _steer = 0.0f;

        bool _available = false;
        float _speed = 0.0f;
        float _clock = 0.0f;

        std::array<SpeedSample, 32> _history{};
        std::size_t _historyNext = 0;
        std::size_t _historyCount = 0;

        CruiseState _cruise;
        float _lastCruiseTarget = 0.0f;

        std::array<CruiseEvent, 16> _events{};
        std::size_t _eventHead = 0;
        std::size_t _eventCount = 0;
    };
}
