#pragma once

// Includes
//------------------------------------------------------------------------------
// Third party
#include <SDL.h>

// Forward Declarations
//------------------------------------------------------------------------------
class SDLRenderer;

//------------------------------------------------------------------------------
class SDLTexture
{
public:
    explicit SDLTexture(SDLRenderer& renderer, uint32_t width, uint32_t height);
    ~SDLTexture();

    bool IsValid() const { return mTexture != nullptr; }
    SDL_Texture* GetTexture() { return mTexture; }

private:
    SDL_Texture* mTexture;
};