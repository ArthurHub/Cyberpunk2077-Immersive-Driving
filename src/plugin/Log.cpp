#include "Log.h"

#include <cstdarg>
#include <cstdio>

namespace immersive_driving::logger
{
    namespace
    {
        RED4ext::v1::PluginHandle pluginHandle = nullptr;
        const RED4ext::v1::Sdk* pluginSdk = nullptr;

        enum class Level
        {
            Debug,
            Info,
            Warn,
            Error,
        };

        void write(const Level level, const char* format, va_list args) noexcept
        {
            if (!pluginSdk || !pluginSdk->logger) {
                return;
            }

            char buffer[1024];
            std::vsnprintf(buffer, sizeof(buffer), format, args);

            const auto* sdkLogger = pluginSdk->logger;
            switch (level) {
            case Level::Debug:
                sdkLogger->Debug(pluginHandle, buffer);
                break;
            case Level::Info:
                sdkLogger->Info(pluginHandle, buffer);
                break;
            case Level::Warn:
                sdkLogger->Warn(pluginHandle, buffer);
                break;
            case Level::Error:
                sdkLogger->Error(pluginHandle, buffer);
                break;
            }
        }
    }

    void init(const RED4ext::v1::PluginHandle handle, const RED4ext::v1::Sdk* sdk) noexcept
    {
        pluginHandle = handle;
        pluginSdk = sdk;
    }

    void debug(const char* format, ...) noexcept
    {
        va_list args;
        va_start(args, format);
        write(Level::Debug, format, args);
        va_end(args);
    }

    void info(const char* format, ...) noexcept
    {
        va_list args;
        va_start(args, format);
        write(Level::Info, format, args);
        va_end(args);
    }

    void warn(const char* format, ...) noexcept
    {
        va_list args;
        va_start(args, format);
        write(Level::Warn, format, args);
        va_end(args);
    }

    void error(const char* format, ...) noexcept
    {
        va_list args;
        va_start(args, format);
        write(Level::Error, format, args);
        va_end(args);
    }
}
