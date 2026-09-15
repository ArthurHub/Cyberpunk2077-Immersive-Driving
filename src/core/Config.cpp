#include "Config.h"

#include <algorithm>
#include <cmath>

#include "MathUtil.h"

namespace immersive_driving
{
    namespace
    {
        struct BoolField
        {
            std::string_view name;
            bool Config::*member;
        };

        struct FloatField
        {
            std::string_view name;
            float Config::*member;
            float min;
            float max;
        };

        struct IntField
        {
            std::string_view name;
            int Config::*member;
            int min;
            int max;
        };

        constexpr BoolField BOOL_FIELDS[] = {
            { "enabled", &Config::enabled },
            { "applyToCars", &Config::applyToCars },
            { "applyToBikes", &Config::applyToBikes },
            { "applyToGamepad", &Config::applyToGamepad },
            { "debugLogging", &Config::debugLogging },
            { "steeringSmoothing", &Config::steeringSmoothing },
            { "speedSensitiveSteering", &Config::speedSensitiveSteering },
            { "cruiseEnabled", &Config::cruiseEnabled },
            { "cruiseCancelOnBrake", &Config::cruiseCancelOnBrake },
            { "cruiseCancelOnHandbrake", &Config::cruiseCancelOnHandbrake },
            { "cruiseCancelOnCollision", &Config::cruiseCancelOnCollision },
            { "cruiseUseBrakes", &Config::cruiseUseBrakes },
        };

        constexpr FloatField FLOAT_FIELDS[] = {
            { "steerRiseSec", &Config::steerRiseSec, 0.0f, 5.0f },
            { "fullSteerBelowKmh", &Config::fullSteerBelowKmh, 0.0f, 300.0f },
            { "highSpeedKmh", &Config::highSpeedKmh, 10.0f, 400.0f },
            { "cruiseMinKmh", &Config::cruiseMinKmh, 1.0f, 200.0f },
            { "cruiseChangeRateKmhps", &Config::cruiseChangeRateKmhps, 0.5f, 100.0f },
        };

        constexpr IntField INT_FIELDS[] = {
            { "throttleNormalPct", &Config::throttleNormalPct, 0, 100 },
            { "throttleSportPct", &Config::throttleSportPct, 0, 100 },
            { "throttleGentlePct", &Config::throttleGentlePct, 0, 100 },
            { "brakeNormalPct", &Config::brakeNormalPct, 0, 100 },
            { "brakeSportPct", &Config::brakeSportPct, 0, 100 },
            { "brakeGentlePct", &Config::brakeGentlePct, 0, 100 },
            { "steerNormalPct", &Config::steerNormalPct, 0, 100 },
            { "steerSportPct", &Config::steerSportPct, 0, 100 },
            { "steerGentlePct", &Config::steerGentlePct, 0, 100 },
            { "highSpeedSteerPct", &Config::highSpeedSteerPct, 5, 100 },
            { "cruiseAfterOverride", &Config::cruiseAfterOverride, 0, 1 },
            { "cruiseMaxBrakePct", &Config::cruiseMaxBrakePct, 0, 100 },
            { "cruiseMaxThrottlePct", &Config::cruiseMaxThrottlePct, 5, 100 },
            { "cruiseResponsivenessPct", &Config::cruiseResponsivenessPct, 10, 300 },
        };
    }

    bool Config::setBool(const std::string_view name, const bool value) noexcept
    {
        for (const auto& field : BOOL_FIELDS) {
            if (field.name == name) {
                this->*field.member = value;
                return true;
            }
        }
        return false;
    }

    bool Config::setFloat(const std::string_view name, const float value) noexcept
    {
        if (!std::isfinite(value)) {
            return false;
        }

        for (const auto& field : FLOAT_FIELDS) {
            if (field.name == name) {
                this->*field.member = clamp(value, field.min, field.max);
                return true;
            }
        }
        return false;
    }

    bool Config::setInt(const std::string_view name, const int value) noexcept
    {
        for (const auto& field : INT_FIELDS) {
            if (field.name == name) {
                this->*field.member = std::clamp(value, field.min, field.max);
                return true;
            }
        }
        return false;
    }
}
