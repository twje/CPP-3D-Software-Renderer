#pragma once

// Includes
//------------------------------------------------------------------------------
// Third Party
#include <glm/glm.hpp>

// Core
#include "Core/SDLWrappers/SDLTexture.h"

// System
#include <cstdint>
#include <vector>

// Forward Declarations
//------------------------------------------------------------------------------
struct AppContext;

//------------------------------------------------------------------------------
class ColorBuffer
{
public:
    ColorBuffer(AppContext& context, const glm::uvec2& size);

    bool IsValid() const;
    void Clear(uint32_t color);
    void SetPixel(int32_t x, int32_t y, uint32_t color);
    void Render();

private:
    AppContext& mContext;
    glm::uvec2 mSize;
    SDLTexture mTexture;
    std::vector<uint32_t> mFrameBuffer;
};