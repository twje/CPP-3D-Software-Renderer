#pragma once

// Includes
//------------------------------------------------------------------------------
// Core
#include "Core/Graphics/Vector.h"

// System
#include <cstdint>
#include <string>

//------------------------------------------------------------------------------
struct AppConfig
{
    std::string mWindowTitle = "SDL Application";
    bool mFullscreen = false;
    bool mUseNativeResolution = true;
	Vector2i mWindowSize = { 800, 600 };
    int32_t mMonitorIndex = 0;
};