#include "Clipping.h"

// Includes
//------------------------------------------------------------------------------
// System
#include <algorithm>

//------------------------------------------------------------------------------
std::array<Plane, 6> ComputePerspectiveFrustrumPlanes(const Angle& fov, float near, float far)
{
    /*            
              /|\
            /  | | 
          /\   | |
        /      | |
     P*|-->  <-|*|   ----> +z-axis
        \      | |
          \/   | |
            \  | | 
              \|/
    */
    
    const float halfFovRad = fov.AsRadians() * 0.5f;
    const float cosHalfFov = std::cos(halfFovRad);
    const float sinHalfFov = std::sin(halfFovRad);
	const glm::vec3 origin{ 0.0f, 0.0f, 0.0f };

    std::array<Plane, 6> planes { };
    
    planes[ClippingPlaneIndex(ClippingPlaneType::LEFT)] = {
        origin, {cosHalfFov, 0.0f, sinHalfFov}
    };
    
    planes[ClippingPlaneIndex(ClippingPlaneType::RIGHT)] = {
        origin, {-cosHalfFov, 0.0f, sinHalfFov}
    };
    
    planes[ClippingPlaneIndex(ClippingPlaneType::TOP)] = {
        origin, {0.0f, -cosHalfFov, sinHalfFov}
    };
    
    planes[ClippingPlaneIndex(ClippingPlaneType::BOTTOM)] = {
        origin, {0.0f, cosHalfFov, sinHalfFov}
    };

    planes[ClippingPlaneIndex(ClippingPlaneType::NEAR)] = {
        {0.0f, 0.0f, near}, {0.0f, 0.0f, 1.0f}
    };

    planes[ClippingPlaneIndex(ClippingPlaneType::FAR)] = {
        {0.0f, 0.0f, far}, {0.0f, 0.0f, -1.0f}
    };

	return planes;
}

//------------------------------------------------------------------------------
FrustumClippedPolygon::FrustumClippedPolygon(const std::array<Plane, 6>& planes, const std::array<glm::vec4, 3>& triangleVertices)
	: mPlanes(planes)
    , mVertexCount(triangleVertices.size())
{
    std::transform(triangleVertices.begin(), triangleVertices.end(), mVertices.begin(),
        [](const glm::vec4& v) -> glm::vec3 {
            return glm::vec3(v.x, v.y, v.z);  // Explicit conversion; assuming w is not needed.
    });
}

//------------------------------------------------------------------------------
void FrustumClippedPolygon::ClipWithFrustum()
{
	// Clip against each plane (output is input for the next plane)
	for (size_t i = 0; i < static_cast<size_t>(ClippingPlaneType::COUNT); ++i)
	{
		ClipPolygonAgainstPlane(static_cast<ClippingPlaneType>(i));
	}
}

//------------------------------------------------------------------------------
void FrustumClippedPolygon::ClipPolygonAgainstPlane(ClippingPlaneType planeType)
{
	const Plane& plane = mPlanes[ClippingPlaneIndex(planeType)];
	
    std::array<glm::vec3, kMaxVertices> insideVertices;
	size_t insideVertexCount = 0;

	// Clip the polygon against the plane
	for (size_t i = 0; i < mVertexCount; ++i)
	{
		const glm::vec3& currentVertex = mVertices[i];
		const glm::vec3& nextVertex = mVertices[(i + 1) % mVertexCount];

		const float currentDistance = glm::dot(plane.mNormal, currentVertex - plane.mPoint);
		const float nextDistance = glm::dot(plane.mNormal, nextVertex - plane.mPoint);

		if (currentDistance >= 0.0f)
		{
			insideVertices[insideVertexCount++] = currentVertex;
		}

		// If the current and next vertex are on opposite sides of the plane
		if (currentDistance * nextDistance < 0.0f)
		{
			const float t = currentDistance / (currentDistance - nextDistance);
			insideVertices[insideVertexCount++] = glm::mix(currentVertex, nextVertex, t);
		}
	}

	// Update the polygon with the inside clipped vertices
    std::copy(insideVertices.begin(), insideVertices.begin() + insideVertexCount, mVertices.begin());
	mVertexCount = insideVertexCount;
}