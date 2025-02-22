#pragma once

// Includes
//------------------------------------------------------------------------------
// System
#include <filesystem>
#include <vector>

// Type Alias
//------------------------------------------------------------------------------
namespace fs = std::filesystem;

//------------------------------------------------------------------------------
fs::path ResolveAssetPath(const fs::path& asset);
std::vector<uint32_t> LoadPNGToRGBA(const fs::path& filepath, int32_t& outWidth, int32_t& outHeight);