#pragma once

// Includes
//------------------------------------------------------------------------------
// System
#include <vector>

// Third party
#include <glm/glm.hpp>

// Forward Declarations
//------------------------------------------------------------------------------
struct AppContext;

//------------------------------------------------------------------------------
class ZBuffer
{
public:
    ZBuffer(AppContext& context);

    void Clear();
    void SetDepth(int32_t x, int32_t y, float depth);
    float GetDepth(int32_t x, int32_t y) const;

private:
    std::vector<float> mBuffer;
    glm::ivec2 mSize;
};
