#include "DriveController.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "MathUtil.h"

namespace immersive_driving
{
    namespace
    {
        // Longer frames (pause, loading hitch) are clamped so ramps never jump.
        constexpr float MAX_DELTA_TIME = 0.1f;
        // Smallest span used to derive the steering rate, so small targets still ramp in a sensible time.
        constexpr float MIN_RAMP_SPAN = 0.1f;
        // Pedal value above which the driver is considered to press it.
        constexpr float PEDAL_THRESHOLD = 0.1f;
        // Lean value below which the lean keys are considered released.
        constexpr float LEAN_THRESHOLD = 0.01f;

        // Cruise controller. The gains are tuned against a simple vehicle model (tests/CoreTests.cpp) for vehicles
        // that accelerate between roughly 1 and 7 m/s^2 at cruising speed.
        constexpr float CRUISE_KP = 0.35f;
        constexpr float CRUISE_KI = 0.1f;
        constexpr float INITIAL_HOLD_THROTTLE = 0.25f;
        constexpr float INTEGRAL_BRAKE_RANGE = 0.6f;
        constexpr float BRAKE_DEADBAND = 0.8f;
        constexpr float BRAKE_GAIN = 0.5f;
        constexpr float MIN_BRAKE_SPEED = 2.0f;
        constexpr float THROTTLE_SLEW_PER_SEC = 2.5f;
        constexpr float BRAKE_SLEW_PER_SEC = 1.5f;
        constexpr float REVERSE_SPEED = 0.5f;
        constexpr float TOO_SLOW_SECONDS = 1.5f;

        // A speed drop this large inside the window can only be a crash, even full braking is far below it.
        constexpr float COLLISION_WINDOW_SEC = 0.2f;
        constexpr float COLLISION_SPEED_DROP = 4.0f;

        // Speed limiter: the cruise controller caps the driver's throttle. It looks ahead by the measured acceleration so
        // the throttle eases off before the limit, and after kickdown or a lower limit it brings the car down at this rate.
        constexpr float LIMITER_LOOKAHEAD_SEC = 0.5f;
        constexpr float LIMITER_SLOWDOWN = 2.0f;
        constexpr float ACCELERATION_WINDOW_SEC = 0.2f;
        // Kickdown needs the Sport key held this long, like a hold in the scripts, so a tap that toggles Sport mode does
        // not lift the limit.
        constexpr float KICKDOWN_HOLD_SEC = 0.3f;

        /**
         * The level for the active key: Sport wins over Gentle.
         */
        float levelFor(const TickContext& context, const int normalPct, const int sportPct, const int gentlePct) noexcept
        {
            return percentToFraction(context.sport ? sportPct : (context.gentle ? gentlePct : normalPct));
        }

        bool isKindEnabled(const Config& config, const VehicleKind kind) noexcept
        {
            switch (kind) {
            case VehicleKind::Car:
            case VehicleKind::Unknown:
                return config.applyToCars;
            case VehicleKind::Bike:
                return config.applyToBikes;
            default:
                return false;
            }
        }
    }

    TickResult DriveController::tick(const Config& config, const TickContext& context)
    {
        TickResult result;
        result.output = context.game;

        const float dt = clamp(context.deltaTime, 0.0f, MAX_DELTA_TIME);
        _clock += dt;
        _speed = context.speed;
        _available = config.enabled && context.drivingAllowed && isKindEnabled(config, context.vehicleKind);

        if (!config.enabled || !config.limiterEnabled) {
            cancelLimiter();
        }
        _sportKeyHeldTime = context.sport && context.sportKeyHeld ? _sportKeyHeldTime + dt : 0.0f;

        if (!_available) {
            cancelCruise(CruiseEvent::CancelledUnavailable);

            // The limiter waits, and starts again from the speed the car has when driving is allowed again.
            _limiter.rampTarget = std::numeric_limits<float>::max();
            _limiter.braking = false;
            _limiter.brake = 0.0f;

            // Follow the game so nothing jumps when shaping resumes.
            _throttle = clamp01(context.game.accelerate);
            _steer = clamp(context.game.steer, -1.0f, 1.0f);
            _historyCount = 0;
            return result;
        }

        recordSpeed(context.speed);

        float steerInput = clamp(context.game.steer, -1.0f, 1.0f);
        if (context.usingKeyboard && context.modeKeyHeld && std::fabs(context.game.lean) > LEAN_THRESHOLD) {
            // The Sport and Gentle keys can be the vanilla lean keys, which share a paired stick with
            // steering, so the game scales A/D steering down while they are held. Keys are all or nothing, so dividing
            // by the larger component restores the steering, and the unintended lean is dropped.
            steerInput = clamp(steerInput / std::max(std::fabs(steerInput), std::fabs(context.game.lean)), -1.0f, 1.0f);
            result.output.lean = 0.0f;
        }

        const bool shape = context.usingKeyboard || config.applyToGamepad;
        const float throttleInput = clamp01(context.game.accelerate);
        const float brakeInput = clamp01(context.game.decelerate);
        _throttle = shape ? throttleInput * levelFor(context, config.throttleNormalPct, config.throttleSportPct, config.throttleGentlePct) : throttleInput;
        result.output.accelerate = _throttle;
        result.output.decelerate = shape ? brakeInput * levelFor(context, config.brakeNormalPct, config.brakeSportPct, config.brakeGentlePct) : brakeInput;
        result.output.steer = shapeSteering(config, context, steerInput, dt, shape);

        if (_cruise.active) {
            updateCruise(config, context, dt, result.output);
        } else if (_limiter.active) {
            updateLimiter(config, context, dt, result.output);
        }

        result.write = true;
        return result;
    }

    float DriveController::shapeSteering(const Config& config, const TickContext& context, const float input, const float deltaTime, const bool shape)
    {
        if (!shape) {
            _steer = input;
            return input;
        }

        float limit = levelFor(context, config.steerNormalPct, config.steerSportPct, config.steerGentlePct);
        if (!context.sport && config.speedSensitiveSteering) {
            const float fullSteerBelow = kmhToMps(config.fullSteerBelowKmh);
            const float highSpeed = std::max(kmhToMps(config.highSpeedKmh), fullSteerBelow + 1.0f);
            const float t = clamp01((std::fabs(context.speed) - fullSteerBelow) / (highSpeed - fullSteerBelow));
            limit *= lerp(1.0f, percentToFraction(config.highSpeedSteerPct), t);
        }

        const float target = input * limit;
        if (!config.steeringSmoothing) {
            _steer = target;
            return target;
        }

        // Only turning further in is smoothed. Letting go, easing off and changing direction follow the keys at once.
        if (target * _steer < 0.0f) {
            _steer = 0.0f;
        }
        if (std::fabs(target) <= std::fabs(_steer)) {
            _steer = target;
        } else {
            _steer = moveTowards(_steer, target, rateFor(std::max(std::fabs(target), MIN_RAMP_SPAN), config.steerRiseSec) * deltaTime);
        }
        return _steer;
    }

    void DriveController::updateCruise(const Config& config, const TickContext& context, const float deltaTime, DriveInputs& output)
    {
        auto& cruise = _cruise;
        const float speed = context.speed;
        const float minSpeed = kmhToMps(config.cruiseMinKmh);
        const bool driverAccelerating = context.game.accelerate > PEDAL_THRESHOLD;
        const bool driverBraking = context.game.decelerate > PEDAL_THRESHOLD;

        if (!config.cruiseEnabled) {
            cancelCruise(CruiseEvent::CancelledUnavailable);
            return;
        }
        if (driverBraking && config.cruiseCancelOnBrake) {
            cancelCruise(CruiseEvent::CancelledByBrake);
            return;
        }
        if (config.cruiseCancelOnHandbrake && context.handbrake > 0.5f) {
            cancelCruise(CruiseEvent::CancelledByHandbrake);
            return;
        }
        if (config.cruiseCancelOnCollision && detectCollision()) {
            cancelCruise(CruiseEvent::CancelledByCollision);
            return;
        }
        if (speed < -REVERSE_SPEED) {
            cancelCruise(CruiseEvent::CancelledTooSlow);
            return;
        }
        if (speed < minSpeed * 0.5f) {
            cruise.slowTime += deltaTime;
            if (cruise.slowTime > TOO_SLOW_SECONDS) {
                cancelCruise(CruiseEvent::CancelledTooSlow);
                return;
            }
        } else {
            cruise.slowTime = 0.0f;
        }

        // The driver's pedals always win. Cruise waits and keeps its throttle so releasing a pedal never lurches.
        if (driverAccelerating || driverBraking) {
            cruise.overriding = true;
            cruise.rampTarget = std::max(speed, 0.0f);
            cruise.brake = 0.0f;
            cruise.braking = false;
            if (!driverBraking) {
                output.accelerate = std::max(output.accelerate, cruise.throttle);
            }
            return;
        }

        if (cruise.overriding) {
            cruise.overriding = false;
            cruise.rampTarget = std::max(speed, 0.0f);
            if (config.cruiseAfterOverride == static_cast<int>(AfterOverride::UseNewSpeed)) {
                cruise.target = std::max(speed, minSpeed);
                _lastCruiseTarget = cruise.target;
                pushEvent(CruiseEvent::TargetAdopted);
            }
        }

        const float responsiveness = percentToFraction(config.cruiseResponsivenessPct);
        const float kp = CRUISE_KP * responsiveness;
        const float ki = CRUISE_KI * responsiveness * responsiveness;
        const float maxThrottle = percentToFraction(config.cruiseMaxThrottlePct);
        const float maxBrake = config.cruiseUseBrakes ? percentToFraction(config.cruiseMaxBrakePct) : 0.0f;

        // Move the effective target gradually so speed changes stay comfortable.
        cruise.rampTarget = moveTowards(cruise.rampTarget, cruise.target, kmhToMps(config.cruiseChangeRateKmhps) * deltaTime);
        const float error = cruise.rampTarget - speed;

        const float integralMin = maxBrake > 0.0f ? -INTEGRAL_BRAKE_RANGE : 0.0f;
        cruise.integral = clamp(cruise.integral + ki * error * deltaTime, integralMin, maxThrottle);
        const float command = kp * error + cruise.integral;

        // Brakes latch on once clearly too fast (for example downhill) and stay on until the controller asks for
        // throttle again, so the speed settles on the target instead of hovering at the edge of a deadband.
        if (maxBrake <= 0.0f || speed <= MIN_BRAKE_SPEED || command >= 0.0f) {
            cruise.braking = false;
        } else if (error < -BRAKE_DEADBAND && cruise.throttle < 0.05f) {
            cruise.braking = true;
        }

        const float throttleTarget = clamp(command, 0.0f, maxThrottle);
        const float brakeTarget = cruise.braking ? clamp(-command * BRAKE_GAIN, 0.0f, maxBrake) : 0.0f;

        cruise.throttle = moveTowards(cruise.throttle, throttleTarget, THROTTLE_SLEW_PER_SEC * deltaTime);
        cruise.brake = moveTowards(cruise.brake, brakeTarget, BRAKE_SLEW_PER_SEC * deltaTime);

        output.accelerate = std::max(output.accelerate, cruise.throttle);
        output.decelerate = std::max(output.decelerate, cruise.brake);
    }

    void DriveController::updateLimiter(const Config& config, const TickContext& context, const float deltaTime, DriveInputs& output)
    {
        auto& limiter = _limiter;
        const float speed = context.speed;

        // Kickdown: holding the Sport key lifts the limit. After it is let go, the limit follows the car back down.
        if (config.limiterKickdown && _sportKeyHeldTime >= KICKDOWN_HOLD_SEC) {
            limiter.rampTarget = std::max(speed, limiter.target);
            limiter.braking = false;
            limiter.brake = 0.0f;
            return;
        }

        // Above the limit the target comes down gradually, and never stays above the car when it slows down faster.
        limiter.rampTarget = clamp(moveTowards(limiter.rampTarget, limiter.target, LIMITER_SLOWDOWN * deltaTime), limiter.target, std::max(speed, limiter.target));

        // Anticipating while speeding up eases the throttle off before the limit instead of after it.
        const float predictedSpeed = speed + std::max(estimateAcceleration(), 0.0f) * LIMITER_LOOKAHEAD_SEC;
        const float error = limiter.rampTarget - predictedSpeed;
        const float maxBrake = config.limiterUseBrakes ? percentToFraction(config.limiterMaxBrakePct) : 0.0f;

        // The throttle that holds the limit is only learned at the limit, while the cap holds the car back or the car is
        // too fast. Coming down to the limit keeps it, so it is still right on arrival. Otherwise the driver is in
        // control, and braking left over from a downhill is dropped.
        if (limiter.rampTarget > limiter.target) {
            // Keep it.
        } else if (CRUISE_KP * error + limiter.integral < output.accelerate || error < 0.0f) {
            const float integralMin = maxBrake > 0.0f ? -INTEGRAL_BRAKE_RANGE : 0.0f;
            limiter.integral = clamp(limiter.integral + CRUISE_KI * error * deltaTime, integralMin, 1.0f);
        } else {
            limiter.integral = std::max(limiter.integral, 0.0f);
        }
        const float command = CRUISE_KP * error + limiter.integral;

        // Brakes latch like cruise control's, once clearly over the limit.
        if (maxBrake <= 0.0f || speed <= MIN_BRAKE_SPEED || command >= 0.0f) {
            limiter.braking = false;
        } else if (speed - limiter.rampTarget > BRAKE_DEADBAND) {
            limiter.braking = true;
        }

        const float brakeTarget = limiter.braking ? clamp(-command * BRAKE_GAIN, 0.0f, maxBrake) : 0.0f;
        limiter.brake = moveTowards(limiter.brake, brakeTarget, BRAKE_SLEW_PER_SEC * deltaTime);

        output.accelerate = std::min(output.accelerate, clamp01(command));
        output.decelerate = std::max(output.decelerate, limiter.brake);
    }

    EngageResult DriveController::engageCruise(const Config& config, const float targetSpeed)
    {
        if (!config.enabled || !config.cruiseEnabled) {
            return EngageResult::Disabled;
        }
        if (!_available) {
            return EngageResult::NotDriving;
        }

        const float minSpeed = kmhToMps(config.cruiseMinKmh);
        if (_speed < minSpeed) {
            return EngageResult::TooSlow;
        }

        cancelLimiter();
        _cruise = CruiseState{};
        _cruise.active = true;
        _cruise.target = std::max(std::isfinite(targetSpeed) ? targetSpeed : _speed, minSpeed);
        _cruise.rampTarget = _speed;
        _cruise.integral = INITIAL_HOLD_THROTTLE;
        _cruise.throttle = _throttle;
        _lastCruiseTarget = _cruise.target;
        return EngageResult::Engaged;
    }

    bool DriveController::setCruiseTarget(const Config& config, const float targetSpeed)
    {
        if (!_cruise.active || !std::isfinite(targetSpeed)) {
            return false;
        }

        _cruise.target = std::max(targetSpeed, kmhToMps(config.cruiseMinKmh));
        _lastCruiseTarget = _cruise.target;
        return true;
    }

    bool DriveController::cancelCruise(const CruiseEvent reason)
    {
        if (!_cruise.active) {
            return false;
        }

        _cruise.active = false;
        _cruise.overriding = false;
        _cruise.braking = false;
        if (reason != CruiseEvent::None) {
            pushEvent(reason);
        }
        return true;
    }

    bool DriveController::isCruiseActive() const noexcept
    {
        return _cruise.active;
    }

    float DriveController::getCruiseTarget() const noexcept
    {
        return _cruise.active ? _cruise.target : 0.0f;
    }

    float DriveController::getLastCruiseTarget() const noexcept
    {
        return _lastCruiseTarget;
    }

    EngageResult DriveController::engageLimiter(const Config& config, const float limit)
    {
        if (!config.enabled || !config.limiterEnabled) {
            return EngageResult::Disabled;
        }
        if (!_available) {
            return EngageResult::NotDriving;
        }

        cancelCruise();
        startLimiter(std::max(std::isfinite(limit) ? limit : _speed, kmhToMps(config.cruiseMinKmh)));
        return EngageResult::Engaged;
    }

    bool DriveController::setLimiterTarget(const Config& config, const float limit)
    {
        if (!_limiter.active || !std::isfinite(limit)) {
            return false;
        }

        _limiter.target = std::max(limit, kmhToMps(config.cruiseMinKmh));
        _lastLimiterTarget = _limiter.target;
        return true;
    }

    bool DriveController::cancelLimiter() noexcept
    {
        if (!_limiter.active) {
            return false;
        }

        _limiter.active = false;
        _limiter.braking = false;
        _limiter.brake = 0.0f;
        return true;
    }

    bool DriveController::isLimiterActive() const noexcept
    {
        return _limiter.active;
    }

    float DriveController::getLimiterTarget() const noexcept
    {
        return _limiter.active ? _limiter.target : 0.0f;
    }

    float DriveController::getLastLimiterTarget() const noexcept
    {
        return _lastLimiterTarget;
    }

    void DriveController::startLimiter(const float limit) noexcept
    {
        _limiter = LimiterState{};
        _limiter.active = true;
        _limiter.target = limit;
        _limiter.rampTarget = std::numeric_limits<float>::max();
        _limiter.integral = INITIAL_HOLD_THROTTLE;
        _lastLimiterTarget = limit;
    }

    float DriveController::getSpeed() const noexcept
    {
        return _speed;
    }

    bool DriveController::isAvailable() const noexcept
    {
        return _available;
    }

    CruiseEvent DriveController::popEvent() noexcept
    {
        if (_eventCount == 0) {
            return CruiseEvent::None;
        }

        const auto event = _events[_eventHead];
        _eventHead = (_eventHead + 1) % _events.size();
        --_eventCount;
        return event;
    }

    void DriveController::reset(const bool keepLimiter) noexcept
    {
        const bool limiterActive = keepLimiter && _limiter.active;
        const float limit = _limiter.target;
        const float lastLimit = _lastLimiterTarget;

        *this = DriveController{};
        if (limiterActive) {
            startLimiter(limit);
        }
        _lastLimiterTarget = lastLimit;
    }

    void DriveController::recordSpeed(const float speed) noexcept
    {
        _history[_historyNext] = { _clock, speed };
        _historyNext = (_historyNext + 1) % _history.size();
        _historyCount = std::min(_historyCount + 1, _history.size());
    }

    bool DriveController::detectCollision() const noexcept
    {
        if (_historyCount < 2) {
            return false;
        }

        const auto size = _history.size();
        const auto& newest = _history[(_historyNext + size - 1) % size];
        float highest = newest.speed;
        for (std::size_t i = 1; i < _historyCount; ++i) {
            const auto& sample = _history[(_historyNext + size - 1 - i) % size];
            if (newest.time - sample.time > COLLISION_WINDOW_SEC) {
                break;
            }
            highest = std::max(highest, sample.speed);
        }

        return highest - newest.speed > COLLISION_SPEED_DROP;
    }

    float DriveController::estimateAcceleration() const noexcept
    {
        if (_historyCount < 2) {
            return 0.0f;
        }

        const auto size = _history.size();
        const auto& newest = _history[(_historyNext + size - 1) % size];
        const SpeedSample* oldest = &newest;
        for (std::size_t i = 1; i < _historyCount; ++i) {
            const auto& sample = _history[(_historyNext + size - 1 - i) % size];
            if (newest.time - sample.time > ACCELERATION_WINDOW_SEC) {
                break;
            }
            oldest = &sample;
        }

        const float span = newest.time - oldest->time;
        return span > 0.0f ? (newest.speed - oldest->speed) / span : 0.0f;
    }

    void DriveController::pushEvent(const CruiseEvent event) noexcept
    {
        if (_eventCount == _events.size()) {
            // Drop the oldest, the newest explains the current state best.
            _eventHead = (_eventHead + 1) % _events.size();
            --_eventCount;
        }

        _events[(_eventHead + _eventCount) % _events.size()] = event;
        ++_eventCount;
    }
}
