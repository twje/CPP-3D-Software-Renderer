#pragma once

// Includes
//------------------------------------------------------------------------------
// Third party
#include <glm/glm.hpp>

// Forward Declarations
//------------------------------------------------------------------------------
class ColorBuffer;
class Texture;
class ZBuffer;

//------------------------------------------------------------------------------
void DrawTexturedTriangle(ColorBuffer& colorBuffer, ZBuffer& zbuffer, const std::array<glm::vec4, 3>& vertices, const std::array<glm::vec2, 3>& uvs, const std::array<float, 3>& intensity, const Texture& texture);
void DrawWireframeTriangle(ColorBuffer& colorBuffer, const std::array<glm::vec4, 3>& vertices, uint32_t color);