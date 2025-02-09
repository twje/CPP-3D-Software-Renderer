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

	float CalculateLightIntensity(const std::array<glm::vec4, 3>& transformedVertices)
	{		
		glm::vec3 vectorA = transformedVertices[0];  /*   A   */
		glm::vec3 vectorB = transformedVertices[1];  /*  / \  */
		glm::vec3 vectorC = transformedVertices[2];  /* C---B */

		// Get the vector subtraction of B-A and C-A
		glm::vec3 vectorAB = vectorB - vectorA;
		glm::vec3 vectorAC = vectorC - vectorA;
		vectorAB = glm::normalize(vectorAB);
		vectorAC = glm::normalize(vectorAC);

		// Compute the face normal (using cross product to find perpendicular)
		glm::vec3 normal = glm::cross(vectorAB, vectorAC);
		normal = glm::normalize(normal);

		float intensity = -glm::dot(normal, mDirection);

		return intensity;
	}
	
private:
	glm::vec3 mDirection;
};

//------------------------------------------------------------------------------
uint32_t ApplyLightIntensity(uint32_t color, float intensity);