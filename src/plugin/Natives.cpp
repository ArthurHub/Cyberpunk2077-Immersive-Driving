#include "Natives.h"

#include <algorithm>
#include <cstdint>
#include <initializer_list>
#include <string_view>
#include <utility>

#include <RED4ext/RED4ext.hpp>

#include <Version.h>

#include "DrivingRuntime.h"
#include "Log.h"

namespace immersive_driving::natives
{
    namespace
    {
        using Frame = RED4ext::CStackFrame;
        using Context = RED4ext::IScriptable;

        std::string_view nameOf(const RED4ext::CName& name)
        {
            const char* text = name.ToString();
            return text ? std::string_view(text) : std::string_view();
        }

        void getVersion(Context*, Frame* frame, RED4ext::CString* out, int64_t)
        {
            frame->code++;
            if (out) {
                *out = RED4ext::CString(IMMERSIVE_DRIVING_VERSION_STRING);
            }
        }

        void isReady(Context*, Frame* frame, bool* out, int64_t)
        {
            frame->code++;
            const bool ready = DrivingRuntime::get().isReady();
            if (out) {
                *out = ready;
            }
        }

        void getStatus(Context*, Frame* frame, RED4ext::CString* out, int64_t)
        {
            frame->code++;
            const auto status = DrivingRuntime::get().getStatus();
            if (out) {
                *out = RED4ext::CString(status.c_str());
            }
        }

        void setBool(Context*, Frame* frame, bool* out, int64_t)
        {
            RED4ext::CName name;
            bool value = false;
            RED4ext::GetParameter(frame, &name);
            RED4ext::GetParameter(frame, &value);
            frame->code++;

            const bool known = DrivingRuntime::get().setBool(nameOf(name), value);
            if (!known) {
                logger::warn("Unknown boolean setting '%.*s'", static_cast<int>(nameOf(name).size()), nameOf(name).data());
            }
            if (out) {
                *out = known;
            }
        }

        void setFloat(Context*, Frame* frame, bool* out, int64_t)
        {
            RED4ext::CName name;
            float value = 0.0f;
            RED4ext::GetParameter(frame, &name);
            RED4ext::GetParameter(frame, &value);
            frame->code++;

            const bool known = DrivingRuntime::get().setFloat(nameOf(name), value);
            if (!known) {
                logger::warn("Unknown float setting '%.*s'", static_cast<int>(nameOf(name).size()), nameOf(name).data());
            }
            if (out) {
                *out = known;
            }
        }

        void setInt(Context*, Frame* frame, bool* out, int64_t)
        {
            RED4ext::CName name;
            int32_t value = 0;
            RED4ext::GetParameter(frame, &name);
            RED4ext::GetParameter(frame, &value);
            frame->code++;

            const bool known = DrivingRuntime::get().setInt(nameOf(name), value);
            if (!known) {
                logger::warn("Unknown integer setting '%.*s'", static_cast<int>(nameOf(name).size()), nameOf(name).data());
            }
            if (out) {
                *out = known;
            }
        }

        void setPlayerVehicle(Context*, Frame* frame, void*, int64_t)
        {
            RED4ext::Handle<RED4ext::ISerializable> vehicle;
            int32_t kind = 0;
            RED4ext::GetParameter(frame, &vehicle);
            RED4ext::GetParameter(frame, &kind);
            frame->code++;

            DrivingRuntime::get().setPlayerVehicle(vehicle, static_cast<VehicleKind>(std::clamp(kind, 0, static_cast<int32_t>(VehicleKind::Unsupported))));
        }

        void clearPlayerVehicle(Context*, Frame* frame, void*, int64_t)
        {
            frame->code++;
            DrivingRuntime::get().clearPlayerVehicle();
        }

        void setDrivingAllowed(Context*, Frame* frame, void*, int64_t)
        {
            bool allowed = false;
            RED4ext::GetParameter(frame, &allowed);
            frame->code++;
            DrivingRuntime::get().setDrivingAllowed(allowed);
        }

        void setUsingKeyboard(Context*, Frame* frame, void*, int64_t)
        {
            bool usingKeyboard = true;
            RED4ext::GetParameter(frame, &usingKeyboard);
            frame->code++;
            DrivingRuntime::get().setUsingKeyboard(usingKeyboard);
        }

        void logMessage(Context*, Frame* frame, void*, int64_t)
        {
            RED4ext::CString message;
            RED4ext::GetParameter(frame, &message);
            frame->code++;
            logger::info("%s", message.c_str());
        }

        void setGentle(Context*, Frame* frame, void*, int64_t)
        {
            bool active = false;
            bool keyHeld = false;
            RED4ext::GetParameter(frame, &active);
            RED4ext::GetParameter(frame, &keyHeld);
            frame->code++;
            DrivingRuntime::get().setGentle(active, keyHeld);
        }

        void setSport(Context*, Frame* frame, void*, int64_t)
        {
            bool active = false;
            bool keyHeld = false;
            RED4ext::GetParameter(frame, &active);
            RED4ext::GetParameter(frame, &keyHeld);
            frame->code++;
            DrivingRuntime::get().setSport(active, keyHeld);
        }

        void engageCruise(Context*, Frame* frame, int32_t* out, int64_t)
        {
            float targetSpeed = 0.0f;
            RED4ext::GetParameter(frame, &targetSpeed);
            frame->code++;

            const auto result = DrivingRuntime::get().engageCruise(targetSpeed);
            if (out) {
                *out = static_cast<int32_t>(result);
            }
        }

        void setCruiseTarget(Context*, Frame* frame, bool* out, int64_t)
        {
            float targetSpeed = 0.0f;
            RED4ext::GetParameter(frame, &targetSpeed);
            frame->code++;

            const bool changed = DrivingRuntime::get().setCruiseTarget(targetSpeed);
            if (out) {
                *out = changed;
            }
        }

        void cancelCruise(Context*, Frame* frame, bool* out, int64_t)
        {
            frame->code++;
            const bool wasActive = DrivingRuntime::get().cancelCruise();
            if (out) {
                *out = wasActive;
            }
        }

        void isCruiseActive(Context*, Frame* frame, bool* out, int64_t)
        {
            frame->code++;
            const bool active = DrivingRuntime::get().isCruiseActive();
            if (out) {
                *out = active;
            }
        }

        void getCruiseTarget(Context*, Frame* frame, float* out, int64_t)
        {
            frame->code++;
            const float target = DrivingRuntime::get().getCruiseTarget();
            if (out) {
                *out = target;
            }
        }

        void getLastCruiseTarget(Context*, Frame* frame, float* out, int64_t)
        {
            frame->code++;
            const float target = DrivingRuntime::get().getLastCruiseTarget();
            if (out) {
                *out = target;
            }
        }

        void engageLimiter(Context*, Frame* frame, int32_t* out, int64_t)
        {
            float limit = 0.0f;
            RED4ext::GetParameter(frame, &limit);
            frame->code++;

            const auto result = DrivingRuntime::get().engageLimiter(limit);
            if (out) {
                *out = static_cast<int32_t>(result);
            }
        }

        void setLimiterTarget(Context*, Frame* frame, bool* out, int64_t)
        {
            float limit = 0.0f;
            RED4ext::GetParameter(frame, &limit);
            frame->code++;

            const bool changed = DrivingRuntime::get().setLimiterTarget(limit);
            if (out) {
                *out = changed;
            }
        }

        void cancelLimiter(Context*, Frame* frame, bool* out, int64_t)
        {
            frame->code++;
            const bool wasActive = DrivingRuntime::get().cancelLimiter();
            if (out) {
                *out = wasActive;
            }
        }

        void isLimiterActive(Context*, Frame* frame, bool* out, int64_t)
        {
            frame->code++;
            const bool active = DrivingRuntime::get().isLimiterActive();
            if (out) {
                *out = active;
            }
        }

        void getLimiterTarget(Context*, Frame* frame, float* out, int64_t)
        {
            frame->code++;
            const float target = DrivingRuntime::get().getLimiterTarget();
            if (out) {
                *out = target;
            }
        }

        void getLastLimiterTarget(Context*, Frame* frame, float* out, int64_t)
        {
            frame->code++;
            const float target = DrivingRuntime::get().getLastLimiterTarget();
            if (out) {
                *out = target;
            }
        }

        void getSpeed(Context*, Frame* frame, float* out, int64_t)
        {
            frame->code++;
            const float speed = DrivingRuntime::get().getSpeed();
            if (out) {
                *out = speed;
            }
        }

        void popCruiseEvent(Context*, Frame* frame, int32_t* out, int64_t)
        {
            frame->code++;
            const auto event = DrivingRuntime::get().popCruiseEvent();
            if (out) {
                *out = static_cast<int32_t>(event);
            }
        }

        using Param = std::pair<const char*, const char*>;

        template <typename T>
        void registerGlobal(RED4ext::CRTTISystem* rtti, const char* name, void (*function)(Context*, Frame*, T, int64_t), const char* returnType,
            const std::initializer_list<Param> params)
        {
            auto* global = RED4ext::CGlobalFunction::Create(name, name, function);
            global->flags = { .isNative = true, .isStatic = true };
            if (returnType) {
                global->SetReturnType(returnType);
            }
            for (const auto& [type, paramName] : params) {
                global->AddParam(type, paramName);
            }
            rtti->RegisterFunction(global);
        }

        void registerTypes()
        {}

        void postRegisterTypes()
        {
            auto* rtti = RED4ext::CRTTISystem::Get();

            registerGlobal(rtti, "ImmersiveDriving_GetVersion", &getVersion, "String", {});
            registerGlobal(rtti, "ImmersiveDriving_IsReady", &isReady, "Bool", {});
            registerGlobal(rtti, "ImmersiveDriving_GetStatus", &getStatus, "String", {});
            registerGlobal(rtti, "ImmersiveDriving_Log", &logMessage, nullptr, { { "String", "message" } });
            registerGlobal(rtti, "ImmersiveDriving_SetBool", &setBool, "Bool", { { "CName", "name" }, { "Bool", "value" } });
            registerGlobal(rtti, "ImmersiveDriving_SetFloat", &setFloat, "Bool", { { "CName", "name" }, { "Float", "value" } });
            registerGlobal(rtti, "ImmersiveDriving_SetInt", &setInt, "Bool", { { "CName", "name" }, { "Int32", "value" } });
            registerGlobal(rtti, "ImmersiveDriving_SetPlayerVehicle", &setPlayerVehicle, nullptr, { { "handle:vehicleBaseObject", "vehicle" }, { "Int32", "kind" } });
            registerGlobal(rtti, "ImmersiveDriving_ClearPlayerVehicle", &clearPlayerVehicle, nullptr, {});
            registerGlobal(rtti, "ImmersiveDriving_SetDrivingAllowed", &setDrivingAllowed, nullptr, { { "Bool", "allowed" } });
            registerGlobal(rtti, "ImmersiveDriving_SetUsingKeyboard", &setUsingKeyboard, nullptr, { { "Bool", "usingKeyboard" } });
            registerGlobal(rtti, "ImmersiveDriving_SetGentle", &setGentle, nullptr, { { "Bool", "active" }, { "Bool", "keyHeld" } });
            registerGlobal(rtti, "ImmersiveDriving_SetSport", &setSport, nullptr, { { "Bool", "active" }, { "Bool", "keyHeld" } });
            registerGlobal(rtti, "ImmersiveDriving_EngageCruise", &engageCruise, "Int32", { { "Float", "targetSpeed" } });
            registerGlobal(rtti, "ImmersiveDriving_SetCruiseTarget", &setCruiseTarget, "Bool", { { "Float", "targetSpeed" } });
            registerGlobal(rtti, "ImmersiveDriving_CancelCruise", &cancelCruise, "Bool", {});
            registerGlobal(rtti, "ImmersiveDriving_IsCruiseActive", &isCruiseActive, "Bool", {});
            registerGlobal(rtti, "ImmersiveDriving_GetCruiseTarget", &getCruiseTarget, "Float", {});
            registerGlobal(rtti, "ImmersiveDriving_GetLastCruiseTarget", &getLastCruiseTarget, "Float", {});
            registerGlobal(rtti, "ImmersiveDriving_EngageLimiter", &engageLimiter, "Int32", { { "Float", "limit" } });
            registerGlobal(rtti, "ImmersiveDriving_SetLimiterTarget", &setLimiterTarget, "Bool", { { "Float", "limit" } });
            registerGlobal(rtti, "ImmersiveDriving_CancelLimiter", &cancelLimiter, "Bool", {});
            registerGlobal(rtti, "ImmersiveDriving_IsLimiterActive", &isLimiterActive, "Bool", {});
            registerGlobal(rtti, "ImmersiveDriving_GetLimiterTarget", &getLimiterTarget, "Float", {});
            registerGlobal(rtti, "ImmersiveDriving_GetLastLimiterTarget", &getLastLimiterTarget, "Float", {});
            registerGlobal(rtti, "ImmersiveDriving_GetSpeed", &getSpeed, "Float", {});
            registerGlobal(rtti, "ImmersiveDriving_PopCruiseEvent", &popCruiseEvent, "Int32", {});

            logger::info("Registered native functions");
        }
    }

    void registerAll()
    {
        auto* rtti = RED4ext::CRTTISystem::Get();
        rtti->AddRegisterCallback(registerTypes);
        rtti->AddPostRegisterCallback(postRegisterTypes);
    }
}
