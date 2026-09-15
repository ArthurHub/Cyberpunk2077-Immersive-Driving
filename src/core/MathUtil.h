#pragma once

#include <cmath>

namespace immersive_driving
{
    constexpr float clamp(const float value, const float min, const float max) noexcept
    {
        return value < min ? min : (value > max ? max : value);
    }

    constexpr float clamp01(const float value) noexcept
    {
        return clamp(value, 0.0f, 1.0f);
    }

    constexpr float lerp(const float from, const float to, const float t) noexcept
    {
        return from + (to - from) * t;
    }

    constexpr float kmhToMps(const float kmh) noexcept
    {
        return kmh / 3.6f;
    }

    constexpr float mpsToKmh(const float mps) noexcept
    {
        return mps * 3.6f;
    }

    constexpr float percentToFraction(const int percent) noexcept
    {
        return static_cast<float>(percent) / 100.0f;
    }

    /**
     * Moves the current value towards the target by at most maxDelta.
     */
    inline float moveTowards(const float current, const float target, const float maxDelta) noexcept
    {
        const float delta = target - current;
        if (std::fabs(delta) <= maxDelta) {
            return target;
        }
        return current + (delta > 0.0f ? maxDelta : -maxDelta);
    }

    /**
     * Rate (units per second) needed to travel the distance in the given time. A zero duration means "instant".
     */
    inline float rateFor(const float distance, const float seconds) noexcept
    {
        if (seconds <= 0.001f) {
            return 1.0e6f;
        }
        return std::fabs(distance) / seconds;
    }
}
