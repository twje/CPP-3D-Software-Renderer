#include "Core/SDLWrappers/SDLTexture.h"

// Includes
//------------------------------------------------------------------------------
// Core
#include "Core/SDLWrappers/SDLRenderer.h"

//------------------------------------------------------------------------------
SDLTexture::SDLTexture(SDLRenderer & renderer, glm::uvec2 size)
    : mTexture(nullptr)
{
    mTexture = SDL_CreateTexture(renderer.GetSDLRenderer(),
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        size.x,
        size.y);

    if (!mTexture)
    {
        SDL_Log("Failed to create texture: %s", SDL_GetError());
    }
}

//------------------------------------------------------------------------------
SDLTexture::~SDLTexture()
{
    if (mTexture)
    {
        SDL_DestroyTexture(mTexture);
    }
}