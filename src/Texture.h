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

private:
	std::vector<uint32_t> mData;
	glm::ivec2 mSize;
};