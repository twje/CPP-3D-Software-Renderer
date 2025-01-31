#pragma once

// Includes
//------------------------------------------------------------------------------
// Third party
#include <SDL.h>

// Forward Declarations
//------------------------------------------------------------------------------
class SDLWindow;

//------------------------------------------------------------------------------
class SDLRenderer
{
public:
    explicit SDLRenderer(SDLWindow& window);
    ~SDLRenderer();

    bool IsValid() const { return mRenderer != nullptr; }
    SDL_Renderer* GetSDLRenderer() { return mRenderer; }

    // Delete copy and assignment to prevent accidental re-initialization
    SDLRenderer(const SDLRenderer&) = delete;
    SDLRenderer& operator=(const SDLRenderer&) = delete;

private:
    SDL_Renderer* mRenderer;
};