#pragma once

// Includes
//------------------------------------------------------------------------------
// Third Party
#include <glm/glm.hpp>

// Forward Declarations
//------------------------------------------------------------------------------
class ColorBuffer;

//------------------------------------------------------------------------------
void DrawVerticalLine(ColorBuffer& buffer, int32_t x, int32_t yStart, int32_t yEnd, int32_t color);
void DrawHorizontalLine(ColorBuffer& buffer, int32_t y, int32_t xStart, int32_t xEnd, int32_t color);
void DrawLine(ColorBuffer& buffer, const glm::ivec2& point0, const glm::ivec2& point1, uint32_t color);
void DrawRectangle(ColorBuffer& buffer, int32_t x, int32_t y, int32_t width, int32_t height, int32_t color);