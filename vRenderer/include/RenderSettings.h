#pragma once

#include "json.hpp"
#include <fstream>
#include <cstdint>

// Your RenderSettings struct
struct RenderSettings
{
    enum API
    {
        VULKAN,
        OPENGL
    };

    // Enum serialization (still needed)
    NLOHMANN_JSON_SERIALIZE_ENUM(RenderSettings::API, {
        {RenderSettings::VULKAN, "VULKAN"},
        {RenderSettings::OPENGL, "OPENGL"}
        })

    API api = RenderSettings::API::VULKAN;
    uint32_t backgroundColor = 0xAABBCCFF;
    bool fpsLimit = true;
    int targetFps = 60;

    // Automatically generates to_json/from_json
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(RenderSettings, api, backgroundColor, fpsLimit, targetFps);
};