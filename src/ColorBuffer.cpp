#include "ColorBuffer.h"

// Includes
//------------------------------------------------------------------------------
// Core
#include "Core/AppContext.h"

//------------------------------------------------------------------------------
ColorBuffer::ColorBuffer(AppContext& context, const glm::uvec2& size)
    : mContext(context)
    , mSize(size)
    , mTexture(context.mRenderer, size)
    , mFrameBuffer(size.x* size.y, 0)
{ }

//------------------------------------------------------------------------------
bool ColorBuffer::IsValid() const 
{ 
    return mTexture.IsValid(); 
}

//------------------------------------------------------------------------------
void ColorBuffer::Clear(uint32_t color)
{
    if (color == 0)
    {
        std::memset(mFrameBuffer.data(), 0, mFrameBuffer.size() * sizeof(uint32_t));
    }
    else
    {
        std::fill(mFrameBuffer.begin(), mFrameBuffer.end(), color);
    }
}

//------------------------------------------------------------------------------
void ColorBuffer::SetPixel(int32_t x, int32_t y, uint32_t color)
{
    if (x >= 0 && x < static_cast<int32_t>(mSize.x) && y >= 0 && y < static_cast<int32_t>(mSize.y))
    {
        mFrameBuffer[y * mSize.x + x] = color;
    }
}

//------------------------------------------------------------------------------
void ColorBuffer::Render()
{
    if (mTexture.IsValid())
    {
        SDL_UpdateTexture(mTexture.GetTexture(), nullptr, mFrameBuffer.data(), mSize.x * sizeof(uint32_t));
        SDL_RenderCopy(mContext.mRenderer.GetSDLRenderer(), mTexture.GetTexture(), nullptr, nullptr);
    }
}