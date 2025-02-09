#pragma once

// Includes
//------------------------------------------------------------------------------
// Third party
#include <glm/glm.hpp>

// System
#include <cstdint>

//------------------------------------------------------------------------------
class Face
{
public:
	// Vertex indices
	int32_t a = 0;
	int32_t b = 0;
	int32_t c = 0;
	
	// Texture indices
	int32_t at = 0;
	int32_t bt = 0;
	int32_t ct = 0;

	// Normal indices
	int32_t an = 0;
	int32_t bn = 0;
	int32_t cn = 0;
};

//------------------------------------------------------------------------------
class Triangle
{
public:
	Triangle()
		: mPoints(3)
		, mAverageDepth(0.0f)
		, mColor(0xFFFFFFFF)
	{ }

	void SetPoint(size_t index, const glm::vec2& point)
	{
		assert(index >= 0 && index < 3);
		mPoints[index] = point;
	}

	void SetAverageDepth(float depth)
	{
		mAverageDepth = depth;
	}

	void SetColor(uint32_t color)
	{
		mColor = color;
	}

	const glm::vec2& GetPoint(size_t index) const
	{
		assert(index >= 0 && index < 3);
		return mPoints[index];
	}

	float GetAverageDepth() const
	{
		return mAverageDepth;
	}

	uint32_t GetColor() const
	{
		return mColor;
	}

private:
	std::vector<glm::vec2> mPoints;
	float mAverageDepth;
	uint32_t mColor;
};