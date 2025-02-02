#pragma once

// Includes
//------------------------------------------------------------------------------
// System
#include <filesystem>

// Type Alias
//------------------------------------------------------------------------------
namespace fs = std::filesystem;

//------------------------------------------------------------------------------
fs::path ResolveAssetPath(const fs::path& asset);