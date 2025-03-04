#include "TriangleRasterizer.h"

// Includes
//------------------------------------------------------------------------------
// Application
#include "ColorBuffer.h"
#include "ZBuffer.h"
#include "Texture.h"
#include "GeometryRenderer.h"

// Third party
#include <glm/glm.hpp>

// System
#include <cstdint>

//------------------------------------------------------------------------------
static int32_t EdgeCrossProduct(const glm::ivec2& a, const glm::ivec2& b, const glm::ivec2 point)
{
    return (b.x - a.x) * (point.y - a.y) - (b.y - a.y) * (point.x - a.x);
}

//------------------------------------------------------------------------------
static uint32_t LightApplyIntensity(uint32_t originalColor, float percentageFactor) 
{
    if (percentageFactor < 0) { percentageFactor = 0; }
    if (percentageFactor > 1) { percentageFactor = 1; }

    uint32_t a = (originalColor & 0xFF000000);
    uint32_t r = static_cast<uint32_t>((originalColor & 0x00FF0000) * percentageFactor);
    uint32_t g = static_cast<uint32_t>((originalColor & 0x0000FF00) * percentageFactor);
    uint32_t b = static_cast<uint32_t>((originalColor & 0x000000FF) * percentageFactor);

    uint32_t new_color = a | (r & 0x00FF0000) | (g & 0x0000FF00) | (b & 0x000000FF);

    return new_color;
}


//------------------------------------------------------------------------------
void DrawTexturedTriangle(ColorBuffer& colorBuffer, ZBuffer& zbuffer, const std::array<glm::vec4, 3>& vertices, const std::array<glm::vec2, 3>& uvs, 
                          const std::array<float, 3>& intensity, const Texture& texture)
{
    // Vertex positions (integer screen coordinates)
    glm::ivec2 p0 = vertices[0];
    glm::ivec2 p1 = vertices[1];
    glm::ivec2 p2 = vertices[2];

    // Precompute inverse depth for perspective-correct interpolation
    float invW0 = 1.0f / vertices[0].w;
    float invW1 = 1.0f / vertices[1].w;
    float invW2 = 1.0f / vertices[2].w;

    // Texture coordinates for each vertex
    glm::vec2 uv0 = uvs[0];
    glm::vec2 uv1 = uvs[1];
    glm::vec2 uv2 = uvs[2];

    // Compute inverse area for barycentric interpolation
    float invTriangleArea = 1.0f / static_cast<float>(EdgeCrossProduct(p0, p1, p2));

    // Compute bounding box
    int32_t xMin = std::min({ p0.x, p1.x, p2.x });
    int32_t yMin = std::min({ p0.y, p1.y, p2.y });
    int32_t xMax = std::max({ p0.x, p1.x, p2.x });
    int32_t yMax = std::max({ p0.y, p1.y, p2.y });

    // Precompute edge function step deltas for rasterization
    int deltaEdge0X = (p1.y - p2.y);
    int deltaEdge1X = (p2.y - p0.y);
    int deltaEdge2X = (p0.y - p1.y);
    int deltaEdge0Y = (p2.x - p1.x);
    int deltaEdge1Y = (p0.x - p2.x);
    int deltaEdge2Y = (p1.x - p0.x);

    // Compute edge function values for the top-left pixel
    glm::ivec2 topLeftPixel = { xMin, yMin };
    int32_t edge0 = EdgeCrossProduct(p1, p2, topLeftPixel);
    int32_t edge1 = EdgeCrossProduct(p2, p0, topLeftPixel);
    int32_t edge2 = EdgeCrossProduct(p0, p1, topLeftPixel);

    // Texture size for coordinate clamping
    const glm::ivec2 texSize = texture.GetSize();

    // Loop over the bounding box (rasterization)
    for (int32_t y = yMin; y < yMax; y++)
    {
        int32_t e0 = edge0;
        int32_t e1 = edge1;
        int32_t e2 = edge2;

        for (int32_t x = xMin; x < xMax; x++)
        {
            // Check if the pixel is inside the triangle
            if (e0 >= 0 && e1 >= 0 && e2 >= 0)
            {
                // Compute barycentric weights
                float alpha = e0 * invTriangleArea;
                float beta = e1 * invTriangleArea;
                float gamma = e2 * invTriangleArea;

                // Perspective-correct depth interpolation
                float interpolatedInvW = alpha * invW0 + beta * invW1 + gamma * invW2;
                float depth = 1.0f - interpolatedInvW;

                // Z-buffer test
                float currentDepth = zbuffer.GetDepth(x, y);
                if (depth < currentDepth)
                {
                    // Perspective-correct UV interpolation
                    float u = (alpha * (uv0.x * invW0) + beta * (uv1.x * invW1) + gamma * (uv2.x * invW2)) / interpolatedInvW;
                    float v = (alpha * (uv0.y * invW0) + beta * (uv1.y * invW1) + gamma * (uv2.y * invW2)) / interpolatedInvW;
                    
					float interpolatedIntensity  = (alpha * (intensity[0] * invW0) + beta * (intensity[1] * invW1) + gamma * (intensity[2] * invW2)) / interpolatedInvW;
					interpolatedIntensity = std::max(0.0f, std::min(1.0f, interpolatedIntensity));

                    // Convert UV to texture coordinates (modulo for wrapping)
                    int32_t texX = static_cast<int32_t>(u * (texSize.x - 1)) % texSize.x;
                    int32_t texY = static_cast<int32_t>(v * (texSize.y - 1)) % texSize.y;
                    if (texX < 0) texX += texSize.x;
                    if (texY < 0) texY += texSize.y;

                    // Fetch texel color and render pixel
                    uint32_t color = LightApplyIntensity(texture.GetPixel(texX, texY), interpolatedIntensity);
                    colorBuffer.SetPixel(x, y, color);
                    zbuffer.SetDepth(x, y, depth);
                }
            }

            // Step edge functions in X direction
            e0 += deltaEdge0X;
            e1 += deltaEdge1X;
            e2 += deltaEdge2X;
        }

        // Step edge functions in Y direction
        edge0 += deltaEdge0Y;
        edge1 += deltaEdge1Y;
        edge2 += deltaEdge2Y;
    }
}

//------------------------------------------------------------------------------
void DrawWireframeTriangle(ColorBuffer& colorBuffer, const std::array<glm::vec4, 3>& vertices, uint32_t color)
{
	// Draw triangle edges
	DrawLine(colorBuffer, glm::ivec2(vertices[0]), glm::ivec2(vertices[1]), color);
	DrawLine(colorBuffer, glm::ivec2(vertices[1]), glm::ivec2(vertices[2]), color);
	DrawLine(colorBuffer, glm::ivec2(vertices[2]), glm::ivec2(vertices[0]), color);
}