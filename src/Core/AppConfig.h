#pragma once

// Includes
//------------------------------------------------------------------------------
// Third party
#include <glm/glm.hpp>

// System
#include <cstdint>
#include <string>

//------------------------------------------------------------------------------
struct AppConfig
{
    std::string mWindowTitle = "SDL Application";
    bool mFullscreen = false;
    bool mUseNativeResolution = true;
	glm::ivec2 mWindowSize { 800, 600 };
    int32_t mMonitorIndex = 0;
};