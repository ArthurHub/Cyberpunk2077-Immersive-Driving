#include "DrivingRuntime.h"

#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstring>

#include <Version.h>

#include "Log.h"
#include "VehicleFunctions.h"
#include "core/MathUtil.h"

namespace immersive_driving
{
    namespace
    {
        // Driving input fields of vehicle::BaseObject in patch 2.31 (Cyberpunk2077.exe 3.0.80.51928). The per-tick
        // input update at RVA 0x2E54FC zeroes them and writes each one from its input action, then the update at RVA
        // 0x2E5368 calls UpdateVehicleCameraInput (the hooked function) and hands the vehicle over to consume them.
        constexpr std::ptrdiff_t ACCELERATE_OFFSET = 0x264; // Accelerate, 0..1
        constexpr std::ptrdiff_t DECELERATE_OFFSET = 0x268; // Decelerate, 0..1
        constexpr std::ptrdiff_t HANDBRAKE_OFFSET = 0x26C; // Handbrake, 0..1
        constexpr std::ptrdiff_t FORWARD_AXIS_OFFSET = 0x270; // Accelerate minus Decelerate
        constexpr std::ptrdiff_t STEER_OFFSET = 0x278; // TurnX, -1..1 (positive = right)
        constexpr std::ptrdiff_t LEAN_OFFSET = 0x27C; // LeanFB, -1..1 (positive = forward)

        constexpr auto TICK_LOG_INTERVAL = std::chrono::seconds(1);

        float readFloat(const RED4ext::ISerializable* object, const std::ptrdiff_t offset) noexcept
        {
            float value;
            std::memcpy(&value, reinterpret_cast<const char*>(object) + offset, sizeof(float));
            return value;
        }

        void writeFloat(RED4ext::ISerializable* object, const std::ptrdiff_t offset, const float value) noexcept
        {
            std::memcpy(reinterpret_cast<char*>(object) + offset, &value, sizeof(float));
        }

        bool isSameLiveObject(const RED4ext::WeakHandle<RED4ext::ISerializable>& handle, const RED4ext::ISerializable* object) noexcept
        {
            return object && handle.instance == object && !handle.Expired();
        }

        float sanitize(const float value) noexcept
        {
            return std::isfinite(value) ? value : 0.0f;
        }
    }

    DrivingRuntime& DrivingRuntime::get() noexcept
    {
        static DrivingRuntime runtime;
        return runtime;
    }

    void DrivingRuntime::afterInputUpdate(RED4ext::ISerializable* vehicle) noexcept
    {
        if (!vehicle || vehicle != _playerVehiclePtr.load(std::memory_order_acquire)) {
            return;
        }

        // Game queries run script code, so they happen before taking the lock.
        float speed = 0.0f;
        const bool speedAvailable = vehicle_functions::getCurrentSpeed(vehicle, speed);
        bool autoDrive = false;
        vehicle_functions::isAutoDriveModeEnabled(vehicle, autoDrive);

        const DriveInputs game{ readFloat(vehicle, ACCELERATE_OFFSET), readFloat(vehicle, DECELERATE_OFFSET), readFloat(vehicle, STEER_OFFSET), readFloat(vehicle, LEAN_OFFSET) };
        const float handbrake = readFloat(vehicle, HANDBRAKE_OFFSET);

        std::lock_guard lock(_mutex);
        if (!isSameLiveObject(_playerVehicle, vehicle)) {
            return;
        }

        const auto now = std::chrono::steady_clock::now();
        const float deltaTime = _lastTick == std::chrono::steady_clock::time_point{} ? 0.0f : std::chrono::duration<float>(now - _lastTick).count();
        _lastTick = now;
        _playerTicks.fetch_add(1, std::memory_order_relaxed);

        if (!speedAvailable && _speedAvailable) {
            logger::error("Could not read the vehicle speed, Drive Modes and Cruise Control stays inactive");
        }
        _speedAvailable = speedAvailable;
        _autoDrive = autoDrive;

        TickContext context;
        context.deltaTime = deltaTime;
        context.speed = sanitize(speed);
        context.game = { sanitize(game.accelerate), sanitize(game.decelerate), sanitize(game.steer), sanitize(game.lean) };
        context.handbrake = sanitize(handbrake);
        context.usingKeyboard = _usingKeyboard;
        context.gentle = _gentle;
        context.sport = _sport;
        context.drivingAllowed = _drivingAllowed && !autoDrive && speedAvailable;
        context.vehicleKind = _vehicleKind;

        const auto result = _controller.tick(_config, context);
        _lastGame = context.game;
        _lastResult = result;

        if (result.write) {
            // Keep the combined axis consistent with the pedals, preserving anything else the game mixed into it.
            const float forwardAxis =
                readFloat(vehicle, FORWARD_AXIS_OFFSET) + (result.output.accelerate - context.game.accelerate) - (result.output.decelerate - context.game.decelerate);

            writeFloat(vehicle, ACCELERATE_OFFSET, result.output.accelerate);
            writeFloat(vehicle, DECELERATE_OFFSET, result.output.decelerate);
            writeFloat(vehicle, FORWARD_AXIS_OFFSET, sanitize(forwardAxis));
            writeFloat(vehicle, STEER_OFFSET, result.output.steer);
            writeFloat(vehicle, LEAN_OFFSET, result.output.lean);
        }

        if (_config.debugLogging && now - _lastTickLog >= TICK_LOG_INTERVAL) {
            _lastTickLog = now;
            logTick(context.game, result);
        }
    }

    void DrivingRuntime::setHookInstalled(const bool installed) noexcept
    {
        _hookInstalled.store(installed, std::memory_order_release);
    }

    bool DrivingRuntime::setBool(const std::string_view name, const bool value)
    {
        std::lock_guard lock(_mutex);
        return _config.setBool(name, value);
    }

    bool DrivingRuntime::setFloat(const std::string_view name, const float value)
    {
        std::lock_guard lock(_mutex);
        return _config.setFloat(name, value);
    }

    bool DrivingRuntime::setInt(const std::string_view name, const int value)
    {
        std::lock_guard lock(_mutex);
        return _config.setInt(name, value);
    }

    void DrivingRuntime::setPlayerVehicle(const RED4ext::Handle<RED4ext::ISerializable>& vehicle, const VehicleKind kind)
    {
        std::lock_guard lock(_mutex);
        auto* instance = vehicle.instance;
        if (!instance) {
            return;
        }

        if (instance != _playerVehiclePtr.load(std::memory_order_acquire)) {
            _controller.reset();
            _lastTick = {};
            logger::info("Player is driving a vehicle (kind %d)", static_cast<int>(kind));
        }

        _playerVehicle = vehicle;
        _vehicleKind = kind;
        _playerVehiclePtr.store(instance, std::memory_order_release);
    }

    void DrivingRuntime::clearPlayerVehicle()
    {
        std::lock_guard lock(_mutex);
        if (!_playerVehiclePtr.load(std::memory_order_acquire)) {
            return;
        }

        _playerVehiclePtr.store(nullptr, std::memory_order_release);
        _playerVehicle.Reset();
        _vehicleKind = VehicleKind::Unknown;
        _controller.reset();
        _gentle = false;
        _sport = false;
        _lastTick = {};
        logger::info("Player left the driver seat");
    }

    void DrivingRuntime::setDrivingAllowed(const bool allowed)
    {
        std::lock_guard lock(_mutex);
        _drivingAllowed = allowed;
    }

    void DrivingRuntime::setUsingKeyboard(const bool usingKeyboard)
    {
        std::lock_guard lock(_mutex);
        _usingKeyboard = usingKeyboard;
    }

    void DrivingRuntime::setGentle(const bool active)
    {
        std::lock_guard lock(_mutex);
        _gentle = active;
    }

    void DrivingRuntime::setSport(const bool active)
    {
        std::lock_guard lock(_mutex);
        _sport = active;
    }

    EngageResult DrivingRuntime::engageCruise(const float targetSpeed)
    {
        std::lock_guard lock(_mutex);
        if (!isReady()) {
            return EngageResult::NotReady;
        }
        return _controller.engageCruise(_config, targetSpeed);
    }

    bool DrivingRuntime::setCruiseTarget(const float targetSpeed)
    {
        std::lock_guard lock(_mutex);
        return _controller.setCruiseTarget(_config, targetSpeed);
    }

    bool DrivingRuntime::cancelCruise()
    {
        std::lock_guard lock(_mutex);
        return _controller.cancelCruise();
    }

    bool DrivingRuntime::isCruiseActive()
    {
        std::lock_guard lock(_mutex);
        return _controller.isCruiseActive();
    }

    float DrivingRuntime::getCruiseTarget()
    {
        std::lock_guard lock(_mutex);
        return _controller.getCruiseTarget();
    }

    float DrivingRuntime::getLastCruiseTarget()
    {
        std::lock_guard lock(_mutex);
        return _controller.getLastCruiseTarget();
    }

    float DrivingRuntime::getSpeed()
    {
        std::lock_guard lock(_mutex);
        return _controller.getSpeed();
    }

    CruiseEvent DrivingRuntime::popCruiseEvent()
    {
        std::lock_guard lock(_mutex);
        return _controller.popEvent();
    }

    bool DrivingRuntime::isReady() const noexcept
    {
        return _hookInstalled.load(std::memory_order_acquire);
    }

    std::string DrivingRuntime::getStatus()
    {
        std::lock_guard lock(_mutex);

        char buffer[512];
        std::snprintf(buffer,
            sizeof(buffer),
            "Drive Modes and Cruise Control %s | hook %s, %llu player ticks | vehicle %s kind %d, driving %s%s, speed %s, %s | cruise %s target %.1f km/h | "
            "game %.2f/%.2f/%.2f -> out %.2f/%.2f/%.2f",
            IMMERSIVE_DRIVING_VERSION_STRING,
            _hookInstalled.load() ? "installed" : "NOT installed",
            static_cast<unsigned long long>(_playerTicks.load()),
            _playerVehiclePtr.load() ? "set" : "none",
            static_cast<int>(_vehicleKind),
            _drivingAllowed ? "allowed" : "blocked",
            _autoDrive ? " (autodrive)" : "",
            _speedAvailable ? "ok" : "unavailable",
            _usingKeyboard ? "keyboard" : "gamepad",
            _controller.isCruiseActive() ? "on" : "off",
            mpsToKmh(_controller.getCruiseTarget()),
            _lastGame.accelerate,
            _lastGame.decelerate,
            _lastGame.steer,
            _lastResult.output.accelerate,
            _lastResult.output.decelerate,
            _lastResult.output.steer);
        return buffer;
    }

    void DrivingRuntime::logTick(const DriveInputs& game, const TickResult& result)
    {
        logger::info("speed %.1f km/h | game %.2f/%.2f/%.2f lean %.2f -> out %.2f/%.2f/%.2f lean %.2f (%s) | gentle %d sport %d | cruise %s %.1f km/h",
            mpsToKmh(_controller.getSpeed()),
            game.accelerate,
            game.decelerate,
            game.steer,
            game.lean,
            result.output.accelerate,
            result.output.decelerate,
            result.output.steer,
            result.output.lean,
            result.write ? "written" : "untouched",
            _gentle,
            _sport,
            _controller.isCruiseActive() ? "on" : "off",
            mpsToKmh(_controller.getCruiseTarget()));
    }
}
