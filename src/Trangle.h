#pragma once

// Includes
//------------------------------------------------------------------------------
// Core
#include "Core/Graphics/Vector.h"

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
	{ }

	void SetPoint(size_t index, const Vector2f& point)
	{
		assert(index >= 0 && index < 3);
		mPoints[index] = point;
	}

	const Vector2f& GetPoint(size_t index) const
	{
		assert(index >= 0 && index < 3);
		return mPoints[index];
	}

private:
	std::vector<Vector2f> mPoints;
};