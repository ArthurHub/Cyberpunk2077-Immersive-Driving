#pragma once

/**
 * Native functions called by the redscript side (scripts/Natives.reds declares them).
 */
namespace immersive_driving::natives
{
    /**
     * Queues registration with the RTTI system. Call once while the plugin loads.
     */
    void registerAll();
}
