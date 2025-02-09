#include "Light.h"

//------------------------------------------------------------------------------
uint32_t ApplyLightIntensity(uint32_t color, float intensity)
{
	intensity = std::max(0.0f, std::min(1.0f, intensity));	

	const uint32_t r = (color & 0xFF000000) >> 24;
	const uint32_t g = (color & 0x00FF0000) >> 16;
	const uint32_t b = (color & 0x0000FF00) >> 8;
	const uint32_t a = (color & 0x000000FF) >> 0;

	const uint32_t newR = static_cast<uint32_t>(r * intensity) << 24;
	const uint32_t newG = static_cast<uint32_t>(g * intensity) << 16;
	const uint32_t newB = static_cast<uint32_t>(b * intensity) << 8;

	return newR | newG | newB | a;
}