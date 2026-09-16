#pragma once

#include <string_view>

namespace immersive_driving
{
    enum class AfterOverride : int
    {
        ResumeSetSpeed = 0,
        UseNewSpeed = 1,
    };

    /**
     * Every tunable used by the native side. Field names match the redscript settings class (scripts/Settings.reds),
     * which pushes each value by name whenever Mod Settings changes.
     */
    struct Config
    {
        // General
        bool enabled = true;
        bool applyToCars = true;
        bool applyToBikes = true;
        bool applyToGamepad = false;
        bool debugLogging = false;

        // Keyboard driving levels, in percent of the key's full value: normal, while holding the Sport key, and while
        // holding the Gentle key. Sport wins when both are held.
        int throttleNormalPct = 60;
        int throttleSportPct = 100;
        int throttleGentlePct = 25;
        int brakeNormalPct = 50;
        int brakeSportPct = 100;
        int brakeGentlePct = 25;
        int steerNormalPct = 75;
        int steerSportPct = 100;
        int steerGentlePct = 50;

        // Advanced steering
        bool steeringSmoothing = true;
        float steerRiseSec = 0.3f;
        bool speedSensitiveSteering = true;
        float fullSteerBelowKmh = 40.0f;
        float highSpeedKmh = 140.0f;
        int highSpeedSteerPct = 65;

        // Cruise control
        bool cruiseEnabled = true;
        float cruiseMinKmh = 20.0f;
        float cruiseChangeRateKmhps = 5.0f;
        int cruiseAfterOverride = static_cast<int>(AfterOverride::ResumeSetSpeed);
        bool cruiseCancelOnBrake = true;
        bool cruiseCancelOnHandbrake = true;
        bool cruiseCancelOnCollision = true;
        bool cruiseUseBrakes = true;
        int cruiseMaxBrakePct = 30;
        int cruiseMaxThrottlePct = 100;
        int cruiseResponsivenessPct = 100;

        // Speed limiter. cruiseMinKmh is also the lowest limit.
        bool limiterEnabled = true;
        bool limiterKeepOnExit = true;
        bool limiterKickdown = true;
        bool limiterUseBrakes = true;
        int limiterMaxBrakePct = 30;

        /**
         * Setters used by the native functions. Values are clamped to sane ranges.
         * They return false when the name is unknown for that type.
         */
        bool setBool(std::string_view name, bool value) noexcept;
        bool setFloat(std::string_view name, float value) noexcept;
        bool setInt(std::string_view name, int value) noexcept;
    };
}
