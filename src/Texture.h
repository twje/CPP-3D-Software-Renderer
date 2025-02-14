#pragma once

// Includes
//------------------------------------------------------------------------------
// Core
#include "Core/Utils.h"

// Third party
#include <glm/glm.hpp>

// System
#include <filesystem>

// Type Alias
//------------------------------------------------------------------------------
namespace fs = std::filesystem;

//------------------------------------------------------------------------------
class Texture
{
public:
	Texture(const fs::path& filepath)
		: mData(LoadPNGToRGBA(filepath, mSize.x, mSize.y))
	{ }

	const glm::ivec2& GetSize() const
	{
		return mSize;
	}

	uint32_t GetPixel(int x, int y) const
	{
		return mData[y * mSize.x + x];
	}

private:
	std::vector<uint32_t> mData;
	glm::ivec2 mSize;
};