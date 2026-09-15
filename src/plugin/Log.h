#pragma once

#include <RED4ext/Api/v1/PluginHandle.hpp>
#include <RED4ext/Api/v1/Sdk.hpp>

/**
 * printf-style logging through the RED4ext logger (red4ext/logs/ImmersiveDriving-*.log).
 */
namespace immersive_driving::logger
{
    void init(RED4ext::v1::PluginHandle handle, const RED4ext::v1::Sdk* sdk) noexcept;

    void debug(const char* format, ...) noexcept;
    void info(const char* format, ...) noexcept;
    void warn(const char* format, ...) noexcept;
    void error(const char* format, ...) noexcept;
}
