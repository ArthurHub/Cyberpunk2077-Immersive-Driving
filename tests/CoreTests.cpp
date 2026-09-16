// Tests for the game-independent driving logic. Run through CTest or directly.

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <functional>
#include <limits>
#include <tuple>
#include <utility>
#include <vector>

#include "core/Config.h"
#include "core/DriveController.h"
#include "core/MathUtil.h"

using namespace immersive_driving;

namespace
{
    int failures = 0;
    int checks = 0;

    void check(const bool condition, const char* expression, const char* file, const int line)
    {
        ++checks;
        if (!condition) {
            ++failures;
            std::printf("  FAILED %s:%d: %s\n", file, line, expression);
        }
    }

    void checkNear(const double actual, const double expected, const double tolerance, const char* expression, const char* file, const int line)
    {
        ++checks;
        if (!(std::fabs(actual - expected) <= tolerance)) {
            ++failures;
            std::printf("  FAILED %s:%d: %s = %.4f, expected %.4f +/- %.4f\n", file, line, expression, actual, expected, tolerance);
        }
    }

#define CHECK(expr) check((expr), #expr, __FILE__, __LINE__)
#define CHECK_NEAR(actual, expected, tolerance) checkNear((actual), (expected), (tolerance), #actual, __FILE__, __LINE__)

    constexpr float DT = 1.0f / 60.0f;

    /**
     * Point-mass car: engine force fading towards top speed, rolling and aero drag, lagged engine response.
     */
    struct VehicleModel
    {
        float speed = 0.0f;
        float maxAcceleration = 7.0f;
        float topSpeed = 60.0f;
        float engineLagSec = 0.25f;
        float grade = 0.0f;
        float engine = 0.0f;

        void step(const float throttle, const float brake, const float dt)
        {
            engine += (throttle - engine) * std::min(1.0f, dt / engineLagSec);
            const float drive = maxAcceleration * engine * std::max(0.0f, 1.0f - speed / topSpeed);
            const float drag = 0.25f + 0.0008f * speed * speed;
            const float braking = speed > 0.0f ? brake * 9.0f : 0.0f;
            speed = std::max(0.0f, speed + (drive - drag - grade - braking) * dt);
        }
    };

    struct Rig
    {
        Config config;
        DriveController controller;
        VehicleModel vehicle;
        TickContext context;

        Rig()
        {
            context.deltaTime = DT;
            context.drivingAllowed = true;
            context.vehicleKind = VehicleKind::Car;
        }

        TickResult step()
        {
            context.speed = vehicle.speed;
            const auto result = controller.tick(config, context);
            vehicle.step(result.output.accelerate, result.output.decelerate, DT);
            return result;
        }

        TickResult stepFor(const float seconds)
        {
            TickResult last;
            const int steps = static_cast<int>(seconds / DT);
            for (int i = 0; i < steps; ++i) {
                last = step();
            }
            return last;
        }

        /**
         * Runs for the given time and returns the lowest and highest speed seen after the settle time.
         */
        std::pair<float, float> run(const float seconds, const float settleSeconds = 0.0f)
        {
            float low = std::numeric_limits<float>::max();
            float high = 0.0f;
            const int steps = static_cast<int>(seconds / DT);
            for (int i = 0; i < steps; ++i) {
                step();
                if (static_cast<float>(i) * DT >= settleSeconds) {
                    low = std::min(low, vehicle.speed);
                    high = std::max(high, vehicle.speed);
                }
            }
            return { low, high };
        }
    };

    void testConfig()
    {
        Config config;
        CHECK(config.setFloat("steerRiseSec", 1.5f));
        CHECK_NEAR(config.steerRiseSec, 1.5f, 1e-6);
        CHECK(config.setFloat("steerRiseSec", 1000.0f));
        CHECK_NEAR(config.steerRiseSec, 5.0f, 1e-6);
        CHECK(!config.setFloat("steerRiseSec", std::numeric_limits<float>::quiet_NaN()));
        CHECK(config.setInt("steerGentlePct", -20));
        CHECK(config.steerGentlePct == 0);
        CHECK(config.setInt("throttleSportPct", 150));
        CHECK(config.throttleSportPct == 100);
        CHECK(config.setBool("cruiseUseBrakes", false));
        CHECK(!config.cruiseUseBrakes);
        CHECK(config.setBool("limiterEnabled", false) && config.setBool("limiterKeepOnExit", false) && config.setBool("limiterKickdown", false) &&
            config.setBool("limiterUseBrakes", false));
        CHECK(!config.limiterEnabled && !config.limiterKeepOnExit && !config.limiterKickdown && !config.limiterUseBrakes);
        CHECK(config.setInt("limiterMaxBrakePct", 150));
        CHECK(config.limiterMaxBrakePct == 100);
        CHECK(!config.setBool("doesNotExist", true));
        CHECK(!config.setFloat("enabled", 1.0f));
        CHECK(!config.setInt("steerRiseSec", 1));
        // Removed settings are unknown.
        CHECK(!config.setFloat("throttleRiseSec", 1.0f));
        CHECK(!config.setBool("throttleSmoothing", true));
        CHECK(!config.setInt("slowTurnStrengthPct", 50));
    }

    void testPedalLevels()
    {
        Rig rig;
        rig.config.throttleGentlePct = 30;
        rig.config.brakeGentlePct = 20;
        rig.config.brakeSportPct = 90;

        // Levels apply at once, without build-up or release time.
        rig.context.game.accelerate = 1.0f;
        auto result = rig.step();
        CHECK(result.write);
        CHECK_NEAR(result.output.accelerate, 0.6f, 1e-6);
        rig.context.game.accelerate = 0.0f;
        result = rig.step();
        CHECK_NEAR(result.output.accelerate, 0.0f, 1e-6);

        rig.context.game.decelerate = 1.0f;
        result = rig.step();
        CHECK_NEAR(result.output.decelerate, 0.5f, 1e-6);
        rig.context.sport = true;
        result = rig.step();
        CHECK_NEAR(result.output.decelerate, 0.9f, 1e-6);
        rig.context.sport = false;
        rig.context.gentle = true;
        result = rig.step();
        CHECK_NEAR(result.output.decelerate, 0.2f, 1e-6);

        rig.context.game.decelerate = 0.0f;
        rig.context.game.accelerate = 1.0f;
        result = rig.step();
        CHECK_NEAR(result.output.accelerate, 0.3f, 1e-6);

        // Sport wins over Gentle.
        rig.context.sport = true;
        result = rig.step();
        CHECK_NEAR(result.output.accelerate, 1.0f, 1e-6);
        rig.context.sport = false;
        rig.context.gentle = false;
        result = rig.step();
        CHECK_NEAR(result.output.accelerate, 0.6f, 1e-6);
    }

    void testGamepadPassThrough()
    {
        Rig rig;
        rig.context.usingKeyboard = false;
        rig.context.game = { 0.42f, 0.0f, -0.3f };
        rig.vehicle.speed = 40.0f;

        auto result = rig.step();
        CHECK(result.write);
        CHECK_NEAR(result.output.accelerate, 0.42f, 1e-6);
        CHECK_NEAR(result.output.steer, -0.3f, 1e-6);

        rig.config.applyToGamepad = true;
        result = rig.step();
        CHECK(result.output.accelerate < 0.42f);
    }

    void testDisabledFollowsGame()
    {
        Rig rig;
        rig.config.enabled = false;
        rig.context.game = { 1.0f, 0.0f, 1.0f };
        auto result = rig.step();
        CHECK(!result.write);

        rig.context.drivingAllowed = false;
        rig.config.enabled = true;
        result = rig.step();
        CHECK(!result.write);

        rig.context.drivingAllowed = true;
        rig.context.vehicleKind = VehicleKind::Bike;
        rig.config.applyToBikes = false;
        result = rig.step();
        CHECK(!result.write);

        rig.context.vehicleKind = VehicleKind::Unsupported;
        rig.config.applyToBikes = true;
        result = rig.step();
        CHECK(!result.write);
    }

    void testSteering()
    {
        Rig rig;
        rig.context.game.steer = 1.0f;

        // Builds up to the 75% limit in 0.3 s.
        auto result = rig.stepFor(0.15f);
        CHECK_NEAR(result.output.steer, 0.375f, 0.02f);
        result = rig.stepFor(0.2f);
        CHECK_NEAR(result.output.steer, 0.75f, 1e-4);

        // Sport lifts the limit, releasing it and letting go are instant.
        rig.context.sport = true;
        result = rig.stepFor(0.1f);
        CHECK_NEAR(result.output.steer, 1.0f, 1e-4);
        rig.context.sport = false;
        result = rig.step();
        CHECK_NEAR(result.output.steer, 0.75f, 1e-6);
        rig.context.game.steer = 0.0f;
        result = rig.step();
        CHECK_NEAR(result.output.steer, 0.0f, 1e-6);

        // Changing direction starts again from the center.
        rig.context.game.steer = 1.0f;
        rig.stepFor(0.35f);
        rig.context.game.steer = -1.0f;
        result = rig.step();
        CHECK(result.output.steer <= 0.0f && result.output.steer > -0.1f);
        result = rig.stepFor(0.35f);
        CHECK_NEAR(result.output.steer, -0.75f, 1e-4);

        // Gentle at the normal level steers exactly like normal steering.
        rig.context.gentle = true;
        result = rig.step();
        CHECK_NEAR(result.output.steer, -0.5f, 1e-6);
        rig.config.steerGentlePct = 75;
        result = rig.stepFor(0.35f);
        CHECK_NEAR(result.output.steer, -0.75f, 1e-6);
        rig.config.steerGentlePct = 50;

        // Without smoothing the level applies at once.
        rig.context.gentle = false;
        rig.config.steeringSmoothing = false;
        rig.context.game.steer = 1.0f;
        result = rig.step();
        CHECK_NEAR(result.output.steer, 0.75f, 1e-6);
        rig.config.steeringSmoothing = true;

        // Speed-sensitive steering at and above the high speed reference, for normal and Gentle but not Sport.
        rig.vehicle.speed = kmhToMps(160.0f);
        rig.vehicle.maxAcceleration = 0.0f;
        rig.vehicle.grade = -1.0f;
        result = rig.stepFor(1.0f);
        CHECK_NEAR(result.output.steer, 0.75f * 0.65f, 1e-3);
        rig.context.gentle = true;
        result = rig.step();
        CHECK_NEAR(result.output.steer, 0.5f * 0.65f, 1e-3);
        rig.context.sport = true;
        result = rig.stepFor(0.5f);
        CHECK_NEAR(result.output.steer, 1.0f, 1e-4);
    }

    void testLeanPairing()
    {
        // Vanilla pairs steering with the lean keys, so A plus Left Shift arrives as (-0.71, 0.71).
        const float paired = 1.0f / std::sqrt(2.0f);
        Rig rig;
        rig.config.steeringSmoothing = false;

        rig.context.sport = true;
        rig.context.modeKeyHeld = true;
        rig.context.game = { 0.0f, 0.0f, -paired, paired };
        auto result = rig.step();
        CHECK_NEAR(result.output.steer, -1.0f, 1e-4);
        CHECK_NEAR(result.output.lean, 0.0f, 1e-6);

        rig.context.sport = false;
        rig.context.gentle = true;
        rig.context.game = { 0.0f, 0.0f, paired, -paired };
        result = rig.step();
        CHECK_NEAR(result.output.steer, 0.5f, 1e-4);
        CHECK_NEAR(result.output.lean, 0.0f, 1e-6);

        // The key alone does not steer.
        rig.context.game = { 0.0f, 0.0f, 0.0f, -1.0f };
        result = rig.step();
        CHECK_NEAR(result.output.steer, 0.0f, 1e-6);
        CHECK_NEAR(result.output.lean, 0.0f, 1e-6);

        // A toggled mode with its key released leaves the vanilla lean keys alone.
        rig.context.modeKeyHeld = false;
        rig.context.game = { 0.0f, 0.0f, paired, paired };
        result = rig.step();
        CHECK_NEAR(result.output.steer, 0.5f * paired, 1e-4);
        CHECK_NEAR(result.output.lean, paired, 1e-6);

        // Leaning without the mod's keys held stays vanilla.
        rig.context.gentle = false;
        rig.context.game = { 0.0f, 0.0f, paired, paired };
        result = rig.step();
        CHECK_NEAR(result.output.steer, 0.75f * paired, 1e-4);
        CHECK_NEAR(result.output.lean, paired, 1e-6);

        // Controller sticks are really analog, so they are never changed.
        rig.context.usingKeyboard = false;
        rig.context.sport = true;
        rig.context.modeKeyHeld = true;
        rig.context.game = { 0.0f, 0.0f, 0.5f, 0.5f };
        result = rig.step();
        CHECK_NEAR(result.output.steer, 0.5f, 1e-6);
        CHECK_NEAR(result.output.lean, 0.5f, 1e-6);
    }

    void testCruiseEngageRules()
    {
        Rig rig;
        rig.vehicle.speed = kmhToMps(10.0f);
        rig.step();
        CHECK(rig.controller.engageCruise(rig.config, rig.vehicle.speed) == EngageResult::TooSlow);

        rig.vehicle.speed = 25.0f;
        rig.step();
        rig.config.cruiseEnabled = false;
        CHECK(rig.controller.engageCruise(rig.config, 25.0f) == EngageResult::Disabled);
        rig.config.cruiseEnabled = true;

        rig.context.drivingAllowed = false;
        rig.step();
        CHECK(rig.controller.engageCruise(rig.config, 25.0f) == EngageResult::NotDriving);

        rig.context.drivingAllowed = true;
        rig.step();
        CHECK(rig.controller.engageCruise(rig.config, 25.0f) == EngageResult::Engaged);
        CHECK(rig.controller.isCruiseActive());
        CHECK_NEAR(rig.controller.getCruiseTarget(), 25.0f, 1e-6);

        // Leaving the driver seat cancels and explains why.
        rig.context.drivingAllowed = false;
        rig.step();
        CHECK(!rig.controller.isCruiseActive());
        CHECK(rig.controller.popEvent() == CruiseEvent::CancelledUnavailable);
        CHECK(rig.controller.popEvent() == CruiseEvent::None);
    }

    struct HoldCase
    {
        const char* name;
        float maxAcceleration;
        float topSpeed;
        float engineLagSec;
        float grade;
        float target;
        int responsivenessPct;
    };

    void testCruiseHoldsSpeed()
    {
        const HoldCase cases[] = {
            { "sports car", 9.0f, 75.0f, 0.2f, 0.0f, 30.0f, 100 },
            { "city car", 6.0f, 55.0f, 0.25f, 0.0f, 22.0f, 100 },
            { "heavy truck", 2.5f, 42.0f, 0.4f, 0.0f, 25.0f, 100 },
            { "motorcycle", 10.0f, 80.0f, 0.15f, 0.0f, 35.0f, 100 },
            { "uphill", 6.0f, 55.0f, 0.25f, 0.8f, 22.0f, 100 },
            { "downhill", 6.0f, 55.0f, 0.25f, -1.5f, 22.0f, 100 },
            { "steep downhill", 6.0f, 55.0f, 0.25f, -3.0f, 22.0f, 100 },
            { "low responsiveness", 6.0f, 55.0f, 0.25f, 0.0f, 22.0f, 40 },
            { "high responsiveness, slow engine", 6.0f, 55.0f, 0.8f, 0.0f, 22.0f, 200 },
        };

        for (const auto& c : cases) {
            std::printf("  cruise hold: %s\n", c.name);
            Rig rig;
            rig.vehicle.maxAcceleration = c.maxAcceleration;
            rig.vehicle.topSpeed = c.topSpeed;
            rig.vehicle.engineLagSec = c.engineLagSec;
            rig.vehicle.grade = c.grade;
            rig.vehicle.speed = c.target;
            rig.config.cruiseResponsivenessPct = c.responsivenessPct;

            rig.step();
            CHECK(rig.controller.engageCruise(rig.config, c.target) == EngageResult::Engaged);

            // Small transient after engaging, then tight.
            const auto [earlyLow, earlyHigh] = rig.run(12.0f);
            CHECK(earlyLow > c.target - 1.0f);
            CHECK(earlyHigh < c.target + 2.0f);
            const auto [low, high] = rig.run(20.0f);
            CHECK_NEAR(low, c.target, 0.4f);
            CHECK_NEAR(high, c.target, 0.4f);
            CHECK(rig.controller.isCruiseActive());
        }
    }

    void testCruiseTargetChanges()
    {
        Rig rig;
        rig.vehicle.speed = 20.0f;
        rig.step();
        CHECK(rig.controller.engageCruise(rig.config, 20.0f) == EngageResult::Engaged);
        rig.run(5.0f);

        CHECK(rig.controller.setCruiseTarget(rig.config, 30.0f));
        auto [low, high] = rig.run(25.0f, 15.0f);
        CHECK_NEAR(low, 30.0f, 0.6f);
        CHECK_NEAR(high, 30.0f, 0.6f);

        // Slowing down must not undershoot much.
        CHECK(rig.controller.setCruiseTarget(rig.config, 15.0f));
        std::tie(low, high) = rig.run(30.0f, 4.0f);
        CHECK(low > 14.0f);
        CHECK_NEAR(rig.vehicle.speed, 15.0f, 0.4f);
        CHECK(rig.controller.isCruiseActive());

        // Targets below the minimum clamp to it.
        CHECK(rig.controller.setCruiseTarget(rig.config, 1.0f));
        CHECK_NEAR(rig.controller.getCruiseTarget(), kmhToMps(rig.config.cruiseMinKmh), 1e-4);
    }

    void testCruiseCancels()
    {
        {
            Rig rig;
            rig.vehicle.speed = 25.0f;
            rig.step();
            rig.controller.engageCruise(rig.config, 25.0f);
            rig.run(2.0f);
            rig.context.game.decelerate = 1.0f;
            rig.step();
            CHECK(!rig.controller.isCruiseActive());
            CHECK(rig.controller.popEvent() == CruiseEvent::CancelledByBrake);
        }
        {
            Rig rig;
            rig.vehicle.speed = 25.0f;
            rig.step();
            rig.controller.engageCruise(rig.config, 25.0f);
            rig.context.handbrake = 1.0f;
            rig.step();
            CHECK(!rig.controller.isCruiseActive());
            CHECK(rig.controller.popEvent() == CruiseEvent::CancelledByHandbrake);
        }
        {
            Rig rig;
            rig.vehicle.speed = 25.0f;
            rig.step();
            rig.controller.engageCruise(rig.config, 25.0f);
            rig.run(1.0f);
            rig.vehicle.speed = 12.0f;
            rig.step();
            CHECK(!rig.controller.isCruiseActive());
            CHECK(rig.controller.popEvent() == CruiseEvent::CancelledByCollision);
        }
        {
            // Blocked by traffic: gives up after staying far below the minimum speed.
            Rig rig;
            rig.vehicle.speed = 10.0f;
            rig.step();
            rig.controller.engageCruise(rig.config, 10.0f);
            rig.config.cruiseCancelOnCollision = false;
            rig.vehicle.maxAcceleration = 0.0f;
            rig.vehicle.speed = 2.0f;
            rig.stepFor(0.5f);
            CHECK(rig.controller.isCruiseActive());
            rig.stepFor(2.0f);
            CHECK(!rig.controller.isCruiseActive());
            CHECK(rig.controller.popEvent() == CruiseEvent::CancelledTooSlow);
        }
        {
            // The driver's own cancel is not queued as an event.
            Rig rig;
            rig.vehicle.speed = 25.0f;
            rig.step();
            rig.controller.engageCruise(rig.config, 25.0f);
            CHECK(rig.controller.cancelCruise());
            CHECK(!rig.controller.cancelCruise());
            CHECK(rig.controller.popEvent() == CruiseEvent::None);
        }
    }

    void testCruiseOverride()
    {
        for (const bool adopt : { false, true }) {
            Rig rig;
            rig.config.cruiseAfterOverride = static_cast<int>(adopt ? AfterOverride::UseNewSpeed : AfterOverride::ResumeSetSpeed);
            rig.vehicle.speed = 20.0f;
            rig.step();
            CHECK(rig.controller.engageCruise(rig.config, 20.0f) == EngageResult::Engaged);
            rig.run(5.0f);

            // Overtake.
            rig.context.game.accelerate = 1.0f;
            rig.context.sport = true;
            rig.stepFor(4.0f);
            CHECK(rig.vehicle.speed > 26.0f);
            CHECK(rig.controller.isCruiseActive());

            rig.context.game.accelerate = 0.0f;
            rig.context.sport = false;
            const float releaseSpeed = rig.vehicle.speed;
            const auto [low, high] = rig.run(30.0f, 20.0f);
            CHECK(rig.controller.isCruiseActive());
            if (adopt) {
                CHECK(rig.controller.popEvent() == CruiseEvent::TargetAdopted);
                CHECK_NEAR(rig.controller.getCruiseTarget(), releaseSpeed, 0.5f);
                CHECK_NEAR(low, releaseSpeed, 0.8f);
            } else {
                CHECK(rig.controller.popEvent() == CruiseEvent::None);
                CHECK_NEAR(low, 20.0f, 0.6f);
                CHECK_NEAR(high, 20.0f, 0.6f);
            }
        }

        // Braking as an override when braking does not cancel.
        Rig rig;
        rig.config.cruiseCancelOnBrake = false;
        rig.vehicle.speed = 25.0f;
        rig.step();
        rig.controller.engageCruise(rig.config, 25.0f);
        rig.run(3.0f);
        rig.context.game.decelerate = 1.0f;
        const auto result = rig.stepFor(1.0f);
        CHECK(rig.controller.isCruiseActive());
        CHECK_NEAR(result.output.accelerate, 0.0f, 1e-4);
        // Half brake (the default brake limit) for a second.
        CHECK(rig.vehicle.speed < 21.0f);
        rig.context.game.decelerate = 0.0f;
        const auto [low, high] = rig.run(30.0f, 20.0f);
        CHECK_NEAR(low, 25.0f, 0.6f);
        CHECK_NEAR(high, 25.0f, 0.6f);
    }

    void testCruiseWithoutBrakes()
    {
        Rig rig;
        rig.config.cruiseUseBrakes = false;
        rig.vehicle.grade = -2.0f;
        rig.vehicle.speed = 20.0f;
        rig.step();
        rig.controller.engageCruise(rig.config, 20.0f);
        float maxBrake = 0.0f;
        const int steps = static_cast<int>(10.0f / DT);
        for (int i = 0; i < steps; ++i) {
            maxBrake = std::max(maxBrake, rig.step().output.decelerate);
        }
        CHECK_NEAR(maxBrake, 0.0f, 1e-6);
        CHECK(rig.vehicle.speed > 21.0f);
    }

    void testResetClearsState()
    {
        Rig rig;
        rig.vehicle.speed = 25.0f;
        rig.step();
        rig.controller.engageCruise(rig.config, 25.0f);
        rig.controller.reset();
        CHECK(!rig.controller.isCruiseActive());
        CHECK_NEAR(rig.controller.getLastCruiseTarget(), 0.0f, 1e-6);
        CHECK(rig.controller.popEvent() == CruiseEvent::None);
    }

    struct LimitCase
    {
        const char* name;
        float maxAcceleration;
        float topSpeed;
        float engineLagSec;
        float grade;
        float limit;
        // Sport mode switched on (full throttle, key not held) instead of the Default 60% throttle.
        bool sport;
    };

    void testLimiterHoldsLimit()
    {
        const LimitCase cases[] = {
            { "sports car, Sport", 9.0f, 75.0f, 0.2f, 0.0f, 30.0f, true },
            { "sports car", 9.0f, 75.0f, 0.2f, 0.0f, 30.0f, false },
            { "city car", 6.0f, 55.0f, 0.25f, 0.0f, 22.0f, false },
            { "heavy truck, Sport", 2.5f, 42.0f, 0.4f, 0.0f, 25.0f, true },
            { "motorcycle, Sport", 10.0f, 80.0f, 0.15f, 0.0f, 35.0f, true },
            { "low limit, Sport", 9.0f, 75.0f, 0.2f, 0.0f, 8.0f, true },
            { "uphill, Sport", 6.0f, 55.0f, 0.25f, 0.8f, 22.0f, true },
            { "downhill", 6.0f, 55.0f, 0.25f, -1.5f, 22.0f, false },
            { "steep downhill", 6.0f, 55.0f, 0.25f, -3.0f, 22.0f, false },
        };

        for (const auto& c : cases) {
            Rig rig;
            rig.vehicle.maxAcceleration = c.maxAcceleration;
            rig.vehicle.topSpeed = c.topSpeed;
            rig.vehicle.engineLagSec = c.engineLagSec;
            rig.vehicle.grade = c.grade;
            rig.vehicle.speed = 3.0f;
            rig.step();
            CHECK(rig.controller.engageLimiter(rig.config, c.limit) == EngageResult::Engaged);

            // The same car without a limiter, to check the limiter does not hold it back on the way up.
            Rig free;
            free.vehicle = rig.vehicle;

            rig.context.game.accelerate = 1.0f;
            rig.context.sport = c.sport;
            free.context.game.accelerate = 1.0f;
            free.context.sport = c.sport;

            float reachLimited = -1.0f;
            float reachFree = -1.0f;
            float highest = 0.0f;
            const int steps = static_cast<int>(40.0f / DT);
            for (int i = 0; i < steps; ++i) {
                rig.step();
                free.step();
                const float time = static_cast<float>(i) * DT;
                if (reachLimited < 0.0f && rig.vehicle.speed >= c.limit - 0.5f) {
                    reachLimited = time;
                }
                if (reachFree < 0.0f && free.vehicle.speed >= c.limit - 0.5f) {
                    reachFree = time;
                }
                highest = std::max(highest, rig.vehicle.speed);
            }
            const auto [low, high] = rig.run(20.0f);
            std::printf("  limiter hold: %-20s reach %.2f s (free %.2f s), highest %+.2f, settled %+.2f..%+.2f\n",
                c.name,
                reachLimited,
                reachFree,
                highest - c.limit,
                low - c.limit,
                high - c.limit);

            // Eases in shortly before the limit, barely passes it on flat roads, and holds it.
            CHECK(reachLimited >= 0.0f && reachLimited < reachFree + 1.2f);
            CHECK(highest < c.limit + (c.grade < 0.0f ? 1.5f : 0.6f));
            CHECK_NEAR(low, c.limit, 0.3f);
            CHECK_NEAR(high, c.limit, 0.3f);
            CHECK(rig.controller.isLimiterActive());
        }
    }

    void testLimiterEngageRules()
    {
        Rig rig;
        rig.vehicle.speed = 25.0f;
        rig.step();

        rig.config.limiterEnabled = false;
        CHECK(rig.controller.engageLimiter(rig.config, 30.0f) == EngageResult::Disabled);
        rig.config.limiterEnabled = true;
        rig.config.enabled = false;
        CHECK(rig.controller.engageLimiter(rig.config, 30.0f) == EngageResult::Disabled);
        rig.config.enabled = true;

        rig.context.drivingAllowed = false;
        rig.step();
        CHECK(rig.controller.engageLimiter(rig.config, 30.0f) == EngageResult::NotDriving);
        rig.context.drivingAllowed = true;
        rig.step();

        // Changing the limit needs an active limiter, and limits below the minimum speed clamp to it.
        CHECK(!rig.controller.setLimiterTarget(rig.config, 30.0f));
        CHECK(rig.controller.engageLimiter(rig.config, 1.0f) == EngageResult::Engaged);
        CHECK_NEAR(rig.controller.getLimiterTarget(), kmhToMps(rig.config.cruiseMinKmh), 1e-4);
        CHECK(rig.controller.setLimiterTarget(rig.config, 30.0f));
        CHECK_NEAR(rig.controller.getLimiterTarget(), 30.0f, 1e-6);
        CHECK_NEAR(rig.controller.getLastLimiterTarget(), 30.0f, 1e-6);

        // Cruise control and the limiter replace each other, silently.
        CHECK(rig.controller.engageCruise(rig.config, 25.0f) == EngageResult::Engaged);
        CHECK(!rig.controller.isLimiterActive());
        CHECK_NEAR(rig.controller.getLastLimiterTarget(), 30.0f, 1e-6);
        CHECK(rig.controller.engageLimiter(rig.config, 30.0f) == EngageResult::Engaged);
        CHECK(!rig.controller.isCruiseActive());
        CHECK(rig.controller.popEvent() == CruiseEvent::None);

        // Cruise control that cannot engage leaves the limiter on.
        rig.vehicle.speed = 2.0f;
        rig.step();
        CHECK(rig.controller.engageCruise(rig.config, 2.0f) == EngageResult::TooSlow);
        CHECK(rig.controller.isLimiterActive());

        // It waits while driving is not allowed, without touching the inputs.
        rig.context.drivingAllowed = false;
        rig.context.game.accelerate = 1.0f;
        CHECK(!rig.step().write);
        CHECK(rig.controller.isLimiterActive());
        CHECK(rig.controller.popEvent() == CruiseEvent::None);
        rig.context.drivingAllowed = true;
        rig.step();
        CHECK(rig.controller.isLimiterActive());

        // Switching it off in the settings does not.
        rig.config.limiterEnabled = false;
        rig.step();
        CHECK(!rig.controller.isLimiterActive());
        CHECK(!rig.controller.cancelLimiter());
    }

    void testLimiterLeavesDriverInControl()
    {
        Rig rig;
        rig.vehicle.maxAcceleration = 0.0f;
        rig.vehicle.speed = 10.0f;
        rig.step();
        CHECK(rig.controller.engageLimiter(rig.config, 25.0f) == EngageResult::Engaged);

        // Well below the limit the throttle is the driver's.
        rig.context.game.accelerate = 1.0f;
        auto result = rig.stepFor(1.0f);
        CHECK_NEAR(result.output.accelerate, 0.6f, 1e-6);

        // Braking, the handbrake and coasting are left alone and do not switch it off.
        rig.context.game.accelerate = 0.0f;
        rig.context.game.decelerate = 1.0f;
        rig.context.handbrake = 1.0f;
        result = rig.step();
        CHECK_NEAR(result.output.decelerate, 0.5f, 1e-6);
        rig.context.game.decelerate = 0.0f;
        rig.context.handbrake = 0.0f;
        result = rig.stepFor(1.0f);
        CHECK_NEAR(result.output.accelerate, 0.0f, 1e-6);
        CHECK_NEAR(result.output.decelerate, 0.0f, 1e-6);
        CHECK(rig.controller.isLimiterActive());

        // Reversing: the brake key is the reverse throttle, and the limiter never adds any.
        rig.vehicle.speed = -5.0f;
        rig.context.speed = -5.0f;
        rig.context.game.decelerate = 1.0f;
        result = rig.controller.tick(rig.config, rig.context);
        CHECK_NEAR(result.output.decelerate, 0.5f, 1e-6);
        rig.context.game.decelerate = 0.0f;
        result = rig.controller.tick(rig.config, rig.context);
        CHECK_NEAR(result.output.decelerate, 0.0f, 1e-6);
    }

    void testLimiterLowerLimit()
    {
        // Brakes on a flat road, only coasting on a flat road, and brakes downhill.
        for (const auto& [brakes, grade] : { std::pair{ true, 0.0f }, std::pair{ false, 0.0f }, std::pair{ true, -1.5f } }) {
            Rig rig;
            rig.config.limiterUseBrakes = brakes;
            rig.vehicle.grade = grade;
            rig.vehicle.speed = 25.0f;
            rig.step();
            CHECK(rig.controller.engageLimiter(rig.config, 30.0f) == EngageResult::Engaged);
            rig.context.game.accelerate = 1.0f;
            rig.context.sport = true;
            rig.run(15.0f);
            CHECK_NEAR(rig.vehicle.speed, 30.0f, 0.3f);

            // Still holding the throttle: slows down to the new limit without dropping below it.
            CHECK(rig.controller.setLimiterTarget(rig.config, 20.0f));
            float maxBrake = 0.0f;
            float lowest = rig.vehicle.speed;
            const int steps = static_cast<int>(20.0f / DT);
            for (int i = 0; i < steps; ++i) {
                maxBrake = std::max(maxBrake, rig.step().output.decelerate);
                lowest = std::min(lowest, rig.vehicle.speed);
            }
            CHECK(lowest > 19.5f);
            CHECK_NEAR(rig.vehicle.speed, 20.0f, 0.3f);
            if (brakes) {
                CHECK(maxBrake > 0.05f && maxBrake <= 0.3f + 1e-6);
            } else {
                CHECK_NEAR(maxBrake, 0.0f, 1e-6);
            }
        }
    }

    void testLimiterKickdown()
    {
        for (const bool kickdown : { true, false }) {
            Rig rig;
            rig.config.limiterKickdown = kickdown;
            rig.vehicle.speed = 15.0f;
            rig.step();
            CHECK(rig.controller.engageLimiter(rig.config, 20.0f) == EngageResult::Engaged);
            rig.context.game.accelerate = 1.0f;
            rig.run(10.0f);

            // A tap that switches Sport mode on does not lift the limit.
            rig.context.sport = true;
            rig.context.sportKeyHeld = true;
            rig.context.modeKeyHeld = true;
            rig.stepFor(0.2f);
            rig.context.sportKeyHeld = false;
            rig.context.modeKeyHeld = false;
            auto [low, high] = rig.run(3.0f);
            CHECK(high < 20.3f);

            // Holding the Sport key.
            rig.context.sportKeyHeld = true;
            rig.context.modeKeyHeld = true;
            std::tie(low, high) = rig.run(5.0f);
            if (!kickdown) {
                CHECK(high < 20.6f);
                continue;
            }
            CHECK(high > 26.0f);

            // Letting go returns to the limit at a comfortable rate while still holding the throttle.
            rig.context.sport = false;
            rig.context.sportKeyHeld = false;
            rig.context.modeKeyHeld = false;
            const float released = rig.vehicle.speed;
            rig.stepFor(1.0f);
            CHECK(rig.vehicle.speed < released - 1.0f && rig.vehicle.speed > released - 3.5f);
            std::tie(low, high) = rig.run(25.0f);
            CHECK(low > 19.5f);
            CHECK_NEAR(rig.vehicle.speed, 20.0f, 0.3f);
        }
    }

    void testLimiterReset()
    {
        Rig rig;
        rig.vehicle.speed = 25.0f;
        rig.step();
        CHECK(rig.controller.engageLimiter(rig.config, 22.0f) == EngageResult::Engaged);

        // Getting into another car keeps it on, when set to.
        rig.controller.reset(true);
        CHECK(rig.controller.isLimiterActive());
        CHECK_NEAR(rig.controller.getLimiterTarget(), 22.0f, 1e-6);
        rig.vehicle.speed = 15.0f;
        rig.context.game.accelerate = 1.0f;
        rig.context.sport = true;
        const auto [low, high] = rig.run(30.0f, 15.0f);
        CHECK_NEAR(low, 22.0f, 0.3f);
        CHECK_NEAR(high, 22.0f, 0.3f);

        // Otherwise it switches off, and the last limit is still there to resume.
        rig.controller.reset();
        CHECK(!rig.controller.isLimiterActive());
        CHECK_NEAR(rig.controller.getLimiterTarget(), 0.0f, 1e-6);
        CHECK_NEAR(rig.controller.getLastLimiterTarget(), 22.0f, 1e-6);
    }

    struct TestCase
    {
        const char* name;
        std::function<void()> run;
    };
}

int main()
{
    const std::vector<TestCase> tests = {
        { "config", testConfig },
        { "pedal levels", testPedalLevels },
        { "gamepad pass-through", testGamepadPassThrough },
        { "disabled follows game", testDisabledFollowsGame },
        { "steering", testSteering },
        { "lean pairing", testLeanPairing },
        { "cruise engage rules", testCruiseEngageRules },
        { "cruise holds speed", testCruiseHoldsSpeed },
        { "cruise target changes", testCruiseTargetChanges },
        { "cruise cancels", testCruiseCancels },
        { "cruise override", testCruiseOverride },
        { "cruise without brakes", testCruiseWithoutBrakes },
        { "reset", testResetClearsState },
        { "limiter holds limit", testLimiterHoldsLimit },
        { "limiter engage rules", testLimiterEngageRules },
        { "limiter leaves driver in control", testLimiterLeavesDriverInControl },
        { "limiter lower limit", testLimiterLowerLimit },
        { "limiter kickdown", testLimiterKickdown },
        { "limiter reset", testLimiterReset },
    };

    for (const auto& test : tests) {
        const int before = failures;
        std::printf("[ RUN  ] %s\n", test.name);
        test.run();
        std::printf("[ %s ] %s\n", failures == before ? " OK " : "FAIL", test.name);
    }

    std::printf("\n%d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
