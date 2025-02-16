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
class Triangle
{
public:
	Triangle()
		: mVertices(3)
		, mColor(0xFFFFFFFF)
	{ }
	
	void SetVertex(size_t index, const Vertex& vertex)
	{
		mVertices[index] = vertex;
	}

	void SetUV(size_t index, const glm::vec2& uv)
	{
		mVertices[index].mUV = uv;
	}
	
	void SetColor(uint32_t color)
	{
		mColor = color;
	}	

	const Vertex& GetVertex(size_t index) const
	{
		return mVertices[index];
	}

	uint32_t GetColor() const
	{
		return mColor;
	}

private:
	std::vector<Vertex> mVertices;	
	uint32_t mColor;
};