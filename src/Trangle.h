#pragma once

// Includes
//------------------------------------------------------------------------------
// Third party
#include <glm/glm.hpp>

// System
#include <cstdint>

//------------------------------------------------------------------------------
struct Face
{
	std::array<int32_t, 3> mVertexIndicies;
	std::array<int32_t, 3> mTextureIndicies;
	std::array<int32_t, 3> mNormalIndicies;
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

	void SetNormal(size_t index, const glm::vec2& normal)
	{
		assert(index >= 0 && index < 3);
		mNormals[index] = normal;
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

	const glm::vec2& GetNormal(size_t index) const
	{
		assert(index >= 0 && index < 3);
		return mNormals[index];
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
	std::vector<glm::vec2> mNormals;
	float mAverageDepth;
	uint32_t mColor;
};