#include "Matrix.h"

//------------------------------------------------------------------------------
glm::mat4x4 CreateScaleMatrix(const glm::vec3& scale)
{
    return glm::mat4(
        scale.x, 0.0f,    0.0f,    0.0f,
        0.0f,    scale.y, 0.0f,    0.0f,
        0.0f,    0.0f,    scale.z, 0.0f,
        0.0f,    0.0f,    0.0f,    1.0f 
    );
}