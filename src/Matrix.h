#pragma once

// Includes
//------------------------------------------------------------------------------
// Third party
#include <glm/glm.hpp>

//------------------------------------------------------------------------------
glm::mat4 CreateScaleMatrix(const glm::vec3& scale);
glm::mat4 CreateRotateAboutXMatrix(float degrees);
glm::mat4 CreateRotateAboutYMatrix(float degrees);
glm::mat4 CreateRotateAboutZMatrix(float degrees);
glm::mat4 CreateTranslationMatrix(const glm::vec3& translation);
glm::mat4 CreatePerspectiveProjectionMatrix(float fovY, float aspectY, float znear, float zfar);
glm::mat4 CreateLookAt(const glm::vec3& eye, const glm::vec3& target, const glm::vec3& up);
glm::vec4 ProjectVec4(const glm::mat4& projection, const glm::vec4& model);