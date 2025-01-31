#include "Core/SDLWrappers/SDLRenderer.h"

// Includes
//------------------------------------------------------------------------------
// Core
#include "Core/SDLWrappers/SDLWindow.h"

//------------------------------------------------------------------------------
SDLRenderer::SDLRenderer(SDLWindow & window)
    : mRenderer(nullptr)
{
    if (SDL_Window* sdlWindow = window.GetSDLWindow())
    {
        mRenderer = SDL_CreateRenderer(sdlWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!mRenderer)
        {
            SDL_Log("Failed to create renderer: %s", SDL_GetError());
        }
        else
        {
            std::cout << "SDL Renderer created successfully." << std::endl;
        }
    }
}

//------------------------------------------------------------------------------
SDLRenderer::~SDLRenderer()
{
    if (mRenderer)
    {
        SDL_DestroyRenderer(mRenderer);
    }
}