#pragma once

// Includes
//------------------------------------------------------------------------------
// Core
#include "Core/AppConfig.h"
#include "Core/SDLWrappers/SDLWindow.h"
#include "Core/SDLWrappers/SDLRenderer.h"

// Third party
#include <SDL.h>

//------------------------------------------------------------------------------
struct AppContext
{
    SDLWindow mWindow;
    SDLRenderer mRenderer;

    explicit AppContext(const AppConfig& config)
        : mWindow(config)
        , mRenderer(mWindow)
    { }

    int32_t GetWindowWidth() const { return mWindow.GetWindowWidth(); }
    int32_t GetWindowHeight() const { return mWindow.GetWindowHeight(); }

    bool IsValid() const { return mWindow.IsValid() && mRenderer.IsValid(); }
};