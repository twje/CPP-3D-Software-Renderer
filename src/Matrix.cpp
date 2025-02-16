#include "Matrix.h"

// Includes
//------------------------------------------------------------------------------
// Third party
#include <glm/gtc/matrix_transform.hpp>

/*
    Column-major storage (stored by columns in memory), but conceptually, think of multiplying the first column.
    In standard math notation, this corresponds to the first row.

    The transpose function converts standard math notation (row-major) to column-major storage,
    making the multiplication easier to reason about.
*/

//------------------------------------------------------------------------------
glm::mat4 CreateScaleMatrix(const glm::vec3& scale)
{
    return glm::transpose(glm::mat4(
        scale.x, 0.0f,    0.0f,    0.0f,
        0.0f,    scale.y, 0.0f,    0.0f,
        0.0f,    0.0f,    scale.z, 0.0f,
        0.0f,    0.0f,    0.0f,    1.0f
    ));
}

//------------------------------------------------------------------------------
glm::mat4 CreateRotateAboutXMatrix(float degrees)
{
    const float radians = glm::radians(degrees);
    const float cosTheta = glm::cos(radians);
    const float sinTheta = glm::sin(radians);

    return glm::transpose(glm::mat4(
		1.0f, 0.0f,      0.0f,      0.0f,
		0.0f, cosTheta,  -sinTheta,  0.0f,
		0.0f, sinTheta, cosTheta,  0.0f,
		0.0f, 0.0f,      0.0f,      1.0f
    ));
}

//------------------------------------------------------------------------------
glm::mat4 CreateRotateAboutYMatrix(float degrees)
{
    const float radians = glm::radians(degrees);
    const float cosTheta = glm::cos(radians);
    const float sinTheta = glm::sin(radians);

    return glm::transpose(glm::mat4(
        cosTheta, 0.0f, sinTheta, 0.0f,
        0.0f,     1.0f, 0.0f,      0.0f,
        -sinTheta, 0.0f, cosTheta,  0.0f,
        0.0f,     0.0f, 0.0f,      1.0f
    ));
}

//------------------------------------------------------------------------------
glm::mat4 CreateRotateAboutZMatrix(float degrees)
{    
    const float radians = glm::radians(degrees);
    const float cosTheta = glm::cos(radians);
    const float sinTheta = glm::sin(radians);    

    /*
        Rotation about the Z-axis:
        - Assumes a counterclockwise rotation from the X-axis toward the Y-axis when viewed from 
          the origin, looking in the positive Z direction.
        - The sine terms (sinTheta) are negated to correct for SDL's inverted Y-coordinate system. This has the effect 
          of reversing the direction of the rotation from clockwise to counterclockwise.
    */
    return glm::transpose(glm::mat4(
        cosTheta,  -sinTheta, 0.0f, 0.0f,
        sinTheta, cosTheta, 0.0f, 0.0f,
        0.0f,      0.0f,     1.0f, 0.0f,
        0.0f,      0.0f,     0.0f, 1.0f
    ));
}

//------------------------------------------------------------------------------
glm::mat4 CreateTranslationMatrix(const glm::vec3& translation)
{
    return glm::transpose(glm::mat4(
		1.0f, 0.0f, 0.0f, translation.x,
		0.0f, 1.0f, 0.0f, translation.y,
		0.0f, 0.0f, 1.0f, translation.z,
		0.0f, 0.0f, 0.0f, 1.0f
    ));
}

//------------------------------------------------------------------------------
glm::mat4 CreatePerspectiveProjectionMatrix(float fov, float aspect, float znear, float zfar)
{
    const float radians = glm::radians(fov * 0.5f);
	const float fovScaleFactor = 1.0f / glm::tan(radians);
    const float zScaleFactor = zfar / (zfar - znear);

    return glm::transpose(glm::mat4(
        aspect * fovScaleFactor, 0.0f, 0.0f,         0.0f,
        0.0f,    fovScaleFactor, 0.0f,               0.0f,
        0.0f,                    0.0f, zScaleFactor, -znear * zScaleFactor,
        0.0f,    0.0f,           1.0f,               0.0f
    ));
}

//------------------------------------------------------------------------------
glm::mat4 CreateLookAt(const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up)
{
	const glm::vec3 z = glm::normalize(target - eye);       // Forward (z) vector
	const glm::vec3 x = glm::normalize(glm::cross(up, z));  // Right (x) vector
	const glm::vec3 y = glm::cross(z, x);                   // Up (y) vector

    return glm::transpose(glm::mat4(
		x.x,  x.y,  x.z, -glm::dot(x, eye),
		y.x,  y.y,  y.z, -glm::dot(y, eye),
		z.x,  z.y,  z.z, -glm::dot(z, eye),
		0.0f, 0.0f, 0.0f, 1.0f
    ));     
}

//------------------------------------------------------------------------------
glm::vec4 ProjectVec4(const glm::mat4& projection, const glm::vec4& vector)
{
	glm::vec4 result = projection * vector;
	
	// Perform the perspective divide
    result.x /= result.w;
	result.y /= result.w;
    result.z /= result.w;

    return result;
}
