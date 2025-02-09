#pragma once

// Includes
//------------------------------------------------------------------------------
// Third party
#include <glm/glm.hpp>

//------------------------------------------------------------------------------
struct DirectionalLight
{
public:
	DirectionalLight(const glm::vec3& direction)
		: mDirection(glm::normalize(direction))
	{ }

	float CalculateLightIntensity(const glm::vec3& faceNormal)
	{		
		return -glm::dot(faceNormal, mDirection);
	}
	
private:
	glm::vec3 mDirection;
};

//------------------------------------------------------------------------------
uint32_t ApplyLightIntensity(uint32_t color, float intensity);