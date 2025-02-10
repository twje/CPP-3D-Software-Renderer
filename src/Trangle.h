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
	glm::vec2 mPoint;
	glm::vec2 mNormal;
	glm::vec2 mUV;
};

//------------------------------------------------------------------------------
class Triangle
{
public:
	Triangle()
		: mVertices(3)
		, mAverageDepth(0.0f)
		, mColor(0xFFFFFFFF)
	{ }
	
	void SetVertex(size_t index, const Vertex& vertex)
	{
		mVertices[index] = vertex;
	}

	void SetAverageDepth(float depth)
	{
		mAverageDepth = depth;
	}

	void SetColor(uint32_t color)
	{
		mColor = color;
	}	

	float GetAverageDepth() const
	{
		return mAverageDepth;
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
	float mAverageDepth;
	uint32_t mColor;
};