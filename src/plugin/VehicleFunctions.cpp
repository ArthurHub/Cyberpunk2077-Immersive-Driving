#include "VehicleFunctions.h"

#include <atomic>

#include <RED4ext/RTTISystem.hpp>
#include <RED4ext/RTTITypes.hpp>
#include <RED4ext/Scripting/Functions.hpp>
#include <RED4ext/Scripting/Utils.hpp>

#include "Log.h"

namespace immersive_driving::vehicle_functions
{
    namespace
    {
        /**
         * A VehicleObject method looked up once, on first use.
         */
        class VehicleFunction
        {
        public:
            explicit constexpr VehicleFunction(const char* name) noexcept
                : _name(name)
            {}

            RED4ext::CBaseFunction* get() noexcept
            {
                if (_resolved.load(std::memory_order_acquire)) {
                    return _function.load(std::memory_order_acquire);
                }

                auto* rtti = RED4ext::CRTTISystem::Get();
                auto* vehicleClass = rtti ? rtti->GetClass("vehicleBaseObject") : nullptr;
                RED4ext::CBaseFunction* function = vehicleClass ? vehicleClass->GetFunction(_name) : nullptr;

                _function.store(function, std::memory_order_release);
                _resolved.store(true, std::memory_order_release);

                if (function) {
                    logger::info("Resolved VehicleObject.%s", _name);
                } else {
                    logger::warn("VehicleObject.%s was not found", _name);
                }
                return function;
            }

        private:
            const char* _name;
            std::atomic<RED4ext::CBaseFunction*> _function{ nullptr };
            std::atomic<bool> _resolved{ false };
        };

        VehicleFunction getCurrentSpeedFunction("GetCurrentSpeed");
        VehicleFunction isAutoDriveModeEnabledFunction("IsAutoDriveModeEnabled");
    }

    bool getCurrentSpeed(RED4ext::ISerializable* vehicle, float& speed) noexcept
    {
        auto* function = getCurrentSpeedFunction.get();
        if (!vehicle || !function) {
            return false;
        }

        float result = 0.0f;
        if (!RED4ext::ExecuteFunction(vehicle, function, &result)) {
            return false;
        }

        speed = result;
        return true;
    }

    bool isAutoDriveModeEnabled(RED4ext::ISerializable* vehicle, bool& enabled) noexcept
    {
        auto* function = isAutoDriveModeEnabledFunction.get();
        if (!vehicle || !function) {
            return false;
        }

        bool result = false;
        if (!RED4ext::ExecuteFunction(vehicle, function, &result)) {
            return false;
        }

        enabled = result;
        return true;
    }
}
