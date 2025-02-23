#pragma once

// Includes
//------------------------------------------------------------------------------
// Third party
#include <glm/glm.hpp>

// System
#include <cstdint>

//------------------------------------------------------------------------------
struct Vertex
{
	glm::vec4 mPoint;
	glm::vec2 mNormal;
	glm::vec2 mUV;
};

//------------------------------------------------------------------------------
struct Triangle 
{
	std::array<Vertex, 3> mVertices { };
	uint32_t mColor = 0xFFFFFFFF;
};