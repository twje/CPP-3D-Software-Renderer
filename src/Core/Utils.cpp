#include "Core/Utils.h"

//------------------------------------------------------------------------------
fs::path ResolveAssetPath(const fs::path& asset)
{
	return fs::path(RESOURCES_PATH) / asset;
}