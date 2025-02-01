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
	int32_t a = 0;
	int32_t b = 0;
	int32_t c = 0;
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