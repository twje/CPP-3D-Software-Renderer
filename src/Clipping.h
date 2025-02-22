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
std::array<Plane, 6> ComputePerspectiveFrustrumPlanes(const Angle& fov, float near, float far);

//------------------------------------------------------------------------------
class FrustumClippedPolygon
{
    // Maximum number of vertices when clipping a triangle against a frustum
    static constexpr size_t kMaxVertices = 10;

public:
	explicit FrustumClippedPolygon(const std::array<Plane, 6>& planes, const std::array<glm::vec4, 3>& triangleVertices);
    void ClipWithFrustum();

private:
	void ClipPolygonAgainstPlane(ClippingPlaneType planeType);

    const std::array<Plane, 6>& mPlanes;
	std::array<glm::vec3, kMaxVertices> mVertices;
    size_t mVertexCount;
};