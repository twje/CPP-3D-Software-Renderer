#include "GeometryRenderer.h"

// Includes
//------------------------------------------------------------------------------
// Application
#include "ColorBuffer.h"

//------------------------------------------------------------------------------
void DrawVerticalLine(ColorBuffer& buffer, int32_t x, int32_t yStart, int32_t yEnd, int32_t color)
{
    if (yStart > yEnd)
    {
        std::swap(yStart, yEnd);
    }

    for (int32_t y = yStart; y <= yEnd; ++y)
    {
        buffer.SetPixel(x, y, color);
    }
}

//------------------------------------------------------------------------------
void DrawHorizontalLine(ColorBuffer& buffer, int32_t y, int32_t xStart, int32_t xEnd, int32_t color)
{
    if (xStart > xEnd)
    {
        std::swap(xStart, xEnd);
    }

    for (int32_t x = xStart; x <= xEnd; ++x)
    {
        buffer.SetPixel(x, y, color);
    }
}

//------------------------------------------------------------------------------
void DrawLine(ColorBuffer& buffer, const glm::ivec2& point0, const glm::ivec2& point1, uint32_t color)
{
    // Use DDA algorithm to draw a line
    const int32_t deltaX = point1.x - point0.x;
    const int32_t deltaY = point1.y - point0.y;

    // Take the greater of the two deltas
    const int32_t sideLength = abs(deltaX) >= abs(deltaY) ? abs(deltaX) : abs(deltaY);

    const float xInc = deltaX / static_cast<float>(sideLength);
    const float yInc = deltaY / static_cast<float>(sideLength);

    float currentX = static_cast<float>(point0.x);
    float currentY = static_cast<float>(point0.y);

    for (int32_t i = 0; i <= sideLength; i++)
    {
        int32_t pixelX = static_cast<int32_t>(round(currentX));
        int32_t pixelY = static_cast<int32_t>(round(currentY));

        buffer.SetPixel(pixelX, pixelY, color);

        currentX += xInc;
        currentY += yInc;
    }
}

//------------------------------------------------------------------------------
void DrawRectangle(ColorBuffer& buffer, int32_t x, int32_t y, int32_t width, int32_t height, int32_t color)
{
	DrawHorizontalLine(buffer, y, x, x + width, color);
	DrawHorizontalLine(buffer, y + height, x, x + width, color);
	DrawVerticalLine(buffer, x, y, y + height, color);
	DrawVerticalLine(buffer, x + width, y, y + height, color);
}

//------------------------------------------------------------------------------
void DrawFilledRectangle(ColorBuffer& buffer, int32_t x, int32_t y, int32_t width, int32_t height, int32_t color)
{
    for (int32_t i = 0; i <= width; i += 1)
    {
        for (int32_t j = 0; j <= height; j += 1)
        {
            const int32_t currentX = x + i;
            const int32_t currentY = y + j;
            buffer.SetPixel(currentX, currentY, color);
        }
    }
}