#pragma once

// Includes
//------------------------------------------------------------------------------
// System
#include <cstdint>
#include <string>

//------------------------------------------------------------------------------
struct AppConfig
{
    std::string mWindowTitle = "SDL Application";
    bool mFullscreen = false;
    bool mUseNativeResolution = true;
    int32_t mWindowWidth = 800;
    int32_t mWindowHeight = 600;
    int32_t mMonitorIndex = 0;
};