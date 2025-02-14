#include "Core/Utils.h"

#define STB_IMAGE_IMPLEMENTATION

// Inlcludes
//------------------------------------------------------------------------------
// Third party
#include "stb_image.h"

//------------------------------------------------------------------------------
fs::path ResolveAssetPath(const fs::path& asset)
{
	return fs::path(RESOURCES_PATH) / asset;
}

//------------------------------------------------------------------------------
std::vector<uint32_t> LoadPNGToRGBA(const fs::path& filepath, int32_t& outWidth, int32_t& outHeight)
{
    stbi_set_flip_vertically_on_load(1);
    
    int32_t channels;
    unsigned char* imageData = stbi_load(filepath.string().c_str(), &outWidth, &outHeight, &channels, STBI_rgb_alpha);

    if (!imageData) 
    {
        std::cerr << "Failed to load image: " << filepath << std::endl;
        return { };
    }

    std::vector<uint32_t> pixelData(outWidth * outHeight);

    for (int i = 0; i < outWidth * outHeight; ++i)
    {
        uint8_t r = imageData[i * 4 + 0];  // Red
        uint8_t g = imageData[i * 4 + 1];  // Green
        uint8_t b = imageData[i * 4 + 2];  // Blue
        uint8_t a = imageData[i * 4 + 3];  // Alpha

        pixelData[i] = (static_cast<uint32_t>(r) << 24) |
            (static_cast<uint32_t>(g) << 16) |
            (static_cast<uint32_t>(b) << 8) |
            (static_cast<uint32_t>(a));
    }

    stbi_image_free(imageData);
    
    return pixelData;
}