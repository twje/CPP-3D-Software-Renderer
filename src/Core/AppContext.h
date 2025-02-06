#pragma once

// Includes
//------------------------------------------------------------------------------
// Core
#include "Core/AppConfig.h"
#include "Core/SDLWrappers/SDLWindow.h"
#include "Core/SDLWrappers/SDLRenderer.h"

// Third party
#include <SDL.h>
#include <glm/glm.hpp>

//------------------------------------------------------------------------------
struct AppContext
{
    SDLWindow mWindow;
    SDLRenderer mRenderer;

    explicit AppContext(const AppConfig& config)
        : mWindow(config)
        , mRenderer(mWindow)
    { }

	const glm::ivec2& GetWindowSize() const { return mWindow.GetWindowSize(); }
    bool IsValid() const { return mWindow.IsValid() && mRenderer.IsValid(); }
};