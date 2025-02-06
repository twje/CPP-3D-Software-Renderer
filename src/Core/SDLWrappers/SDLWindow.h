#pragma once

// Includes
//------------------------------------------------------------------------------
// Core
#include "Core/AppConfig.h"

// Third party
#include <glm/glm.hpp>
#include <SDL.h>

//------------------------------------------------------------------------------
class SDLWindow
{
public:
    explicit SDLWindow(const AppConfig& config);
    ~SDLWindow();

    SDL_Window* GetSDLWindow() { return mWindow; }
    bool IsValid() const { return mWindow != nullptr; }
	const glm::ivec2& GetWindowSize() const { return mWindowSize; }

    // Delete copy and assignment to prevent accidental re-initialization
    SDLWindow(const SDLWindow&) = delete;
    SDLWindow& operator=(const SDLWindow&) = delete;

private:
    void CreateWindow(const AppConfig& config);

    SDL_Window* mWindow;
    glm::ivec2 mWindowSize;
};