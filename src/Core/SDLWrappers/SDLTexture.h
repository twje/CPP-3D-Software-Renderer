#pragma once

// Includes
//------------------------------------------------------------------------------
// Third party
#include <SDL.h>
#include <glm/glm.hpp>

// Forward Declarations
//------------------------------------------------------------------------------
class SDLRenderer;

//------------------------------------------------------------------------------
class SDLTexture
{
public:
    explicit SDLTexture(SDLRenderer& renderer, glm::uvec2 size);
    ~SDLTexture();

    bool IsValid() const { return mTexture != nullptr; }
    SDL_Texture* GetTexture() { return mTexture; }

private:
    SDL_Texture* mTexture;
};