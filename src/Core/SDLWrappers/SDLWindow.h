#pragma once

// Includes
//------------------------------------------------------------------------------
// Core
#include "Core/AppConfig.h"
#include "Core/Graphics/Vector.h"

// Third party
#include <SDL.h>

//------------------------------------------------------------------------------
class SDLWindow
{
public:
    explicit SDLWindow(const AppConfig& config);
    ~SDLWindow();

    SDL_Window* GetSDLWindow() { return mWindow; }
    bool IsValid() const { return mWindow != nullptr; }
	const Vector2i& GetWindowSize() const { return mWindowSize; }

    // Delete copy and assignment to prevent accidental re-initialization
    SDLWindow(const SDLWindow&) = delete;
    SDLWindow& operator=(const SDLWindow&) = delete;

private:
    void CreateWindow(const AppConfig& config);

    SDL_Window* mWindow;
    Vector2i mWindowSize;    
};