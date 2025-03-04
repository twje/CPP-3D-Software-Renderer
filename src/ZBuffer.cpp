#include "ZBuffer.h"

// Includes
//------------------------------------------------------------------------------
// Core
#include "Core/AppContext.h"

// System
#include <algorithm>

//------------------------------------------------------------------------------
ZBuffer::ZBuffer(AppContext& context)
    : mBuffer(context.GetWindowSize().x * context.GetWindowSize().y, 1.0f)
    , mSize(context.GetWindowSize())
{ }

//------------------------------------------------------------------------------
void ZBuffer::Clear()
{
    std::fill(mBuffer.begin(), mBuffer.end(), 1.0f);
}

//------------------------------------------------------------------------------
void ZBuffer::SetDepth(int32_t x, int32_t y, float depth)
{
    if (x >= 0 && x < mSize.x && y >= 0 && y < mSize.y)
    {
        mBuffer[y * mSize.x + x] = depth;
    }
}

//------------------------------------------------------------------------------
float ZBuffer::GetDepth(int32_t x, int32_t y) const
{
    if (x >= 0 && x < mSize.x && y >= 0 && y < mSize.y)
    {
        return mBuffer[y * mSize.x + x];
    }

    return 1.0f;
}