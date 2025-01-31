#pragma once

// Includes
//------------------------------------------------------------------------------
// Core
#include "Core/AppConfig.h"
#include "Core/SDLWrappers/SDLWindow.h"
#include "Core/SDLWrappers/SDLRenderer.h"

// Third party
#include <SDL.h>
#include "Core/Graphics/Vector.h"

//------------------------------------------------------------------------------
struct AppContext
{
    SDLWindow mWindow;
    SDLRenderer mRenderer;

    explicit AppContext(const AppConfig& config)
        : mWindow(config)
        , mRenderer(mWindow)
    { }

	const Vector2i& GetWindowSize() const { return mWindow.GetWindowSize(); }
    bool IsValid() const { return mWindow.IsValid() && mRenderer.IsValid(); }
};