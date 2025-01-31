#include "Core/SDLWrappers/SDLTexture.h"

// Includes
//------------------------------------------------------------------------------
// Core
#include "Core/SDLWrappers/SDLRenderer.h"

//------------------------------------------------------------------------------
SDLTexture::SDLTexture(SDLRenderer & renderer, uint32_t width, uint32_t height)
    : mTexture(nullptr)
{
    mTexture = SDL_CreateTexture(renderer.GetSDLRenderer(),
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        width,
        height);

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