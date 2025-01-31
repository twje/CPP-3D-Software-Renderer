#include "Core/SDLWrappers/SDLWindow.h"

//------------------------------------------------------------------------------
SDLWindow::SDLWindow(const AppConfig& config)
    : mWindow(nullptr)
    , mWindowSize(config.mWindowSize)
{
    assert(mWindowSize.x > 0 && mWindowSize.y > 0);
    CreateWindow(config);
}

//------------------------------------------------------------------------------
SDLWindow::~SDLWindow()
{
    if (mWindow)
    {
        SDL_DestroyWindow(mWindow);
    }
}

//------------------------------------------------------------------------------
void SDLWindow::CreateWindow(const AppConfig& config)
{
    uint32_t windowFlags = SDL_WINDOW_SHOWN;
    int32_t displayIndex = config.mMonitorIndex;

    if (config.mFullscreen)
    {
        windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    SDL_DisplayMode displayMode;
    if (SDL_GetCurrentDisplayMode(displayIndex, &displayMode) != 0)
    {
        SDL_Log("Failed to get display mode: %s", SDL_GetError());
        return;
    }

    if (config.mUseNativeResolution)
    {
        mWindowSize.x = displayMode.w;
        mWindowSize.y = displayMode.h;
    }

    int32_t posX = SDL_WINDOWPOS_CENTERED_DISPLAY(displayIndex);
    int32_t posY = SDL_WINDOWPOS_CENTERED_DISPLAY(displayIndex);

    mWindow = SDL_CreateWindow(
        config.mWindowTitle.c_str(),
        posX, posY,
        mWindowSize.x, mWindowSize.y,
        windowFlags);

    if (mWindow)
    {
        std::cout << "SDL Window created successfully on monitor " << displayIndex << "." << std::endl;
        std::cout << "Window Size: " << mWindowSize.x << "x" << mWindowSize.y << std::endl;
        return;
    }

    SDL_Log("Failed to create window: %s", SDL_GetError());    
}