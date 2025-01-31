#pragma once

// Includes
//------------------------------------------------------------------------------
// Core
#include "Core/Graphics/Vector.h"

// Third party
#include <SDL.h>

// Forward Declarations
//------------------------------------------------------------------------------
class SDLRenderer;

//------------------------------------------------------------------------------
class SDLTexture
{
public:
    explicit SDLTexture(SDLRenderer& renderer, Vector2u size);
    ~SDLTexture();

    bool IsValid() const { return mTexture != nullptr; }
    SDL_Texture* GetTexture() { return mTexture; }

private:
    SDL_Texture* mTexture;
};