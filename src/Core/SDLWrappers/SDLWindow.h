#pragma once

// Includes
//------------------------------------------------------------------------------
// Core
#include "Core/AppConfig.h"

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
    uint32_t GetWindowWidth() const { return mWindowWidth; }
    uint32_t GetWindowHeight() const { return mWindowHeight; }

    // Delete copy and assignment to prevent accidental re-initialization
    SDLWindow(const SDLWindow&) = delete;
    SDLWindow& operator=(const SDLWindow&) = delete;

private:
    void CreateWindow(const AppConfig& config);

    SDL_Window* mWindow;
    uint32_t mWindowWidth;
    uint32_t mWindowHeight;
};