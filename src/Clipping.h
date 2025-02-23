#pragma once

// Includes
//------------------------------------------------------------------------------
// Third party
#include <glm/glm.hpp>

// Core
#include "Core/Angle.h"

// System
#include <cstdint>
#include <array>
#include <vector>

//------------------------------------------------------------------------------
enum class ClippingPlaneType : uint8_t
{
    NEAR,
    FAR,
    LEFT,
    RIGHT,
    TOP,
    BOTTOM,
	COUNT,
};

//------------------------------------------------------------------------------
struct Plane
{
	glm::vec3 mPoint;
	glm::vec3 mNormal;
};

//------------------------------------------------------------------------------
constexpr size_t ClippingPlaneIndex(ClippingPlaneType type)
{
    return static_cast<size_t>(type);
}

//------------------------------------------------------------------------------
std::array<Plane, 6> ComputePerspectiveFrustrumPlanes(const Angle& fovX, const Angle& fovY, float near, float far);
std::vector<std::array<glm::vec4, 3>> ClipWithFrustum(const std::array<Plane, 6>& planes, const std::array<glm::vec4, 3>& triangleVertices);