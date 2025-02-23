#include "Clipping.h"

// Includes
//------------------------------------------------------------------------------
// System
#include <algorithm>

//------------------------------------------------------------------------------
std::array<Plane, 6> ComputePerspectiveFrustrumPlanes(const Angle& fovX, const Angle& fovY, float near, float far)
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
    
    const float halfFovRadX = fovX.AsRadians() * 0.5f;
    const float cosHalfFovX = std::cos(halfFovRadX);
    const float sinHalfFovX = std::sin(halfFovRadX);
    
    const float halfFovRadY = fovY.AsRadians() * 0.5f;
    const float cosHalfFovY = std::cos(halfFovRadY);
    const float sinHalfFovY = std::sin(halfFovRadY);

	const glm::vec3 origin{ 0.0f, 0.0f, 0.0f };

    std::array<Plane, 6> planes { };
    
    planes[ClippingPlaneIndex(ClippingPlaneType::LEFT)] = {
        origin, {cosHalfFovX, 0.0f, sinHalfFovX}
    };
    
    planes[ClippingPlaneIndex(ClippingPlaneType::RIGHT)] = {
        origin, {-cosHalfFovX, 0.0f, sinHalfFovX}
    };
    
    planes[ClippingPlaneIndex(ClippingPlaneType::TOP)] = {
        origin, {0.0f, -cosHalfFovY, sinHalfFovY}
    };
    
    planes[ClippingPlaneIndex(ClippingPlaneType::BOTTOM)] = {
        origin, {0.0f, cosHalfFovY, sinHalfFovY}
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
std::vector<std::array<glm::vec4, 3>> FrustumClippedPolygon::ClipWithFrustum()
{
	// Clip against each plane (output is input for the next plane)
	for (size_t i = 0; i < static_cast<size_t>(ClippingPlaneType::COUNT); ++i)
	{
		ClipPolygonAgainstPlane(static_cast<ClippingPlaneType>(i));
	}

	// Return the clipped triangles
	std::vector<std::array<glm::vec4, 3>> clippedTriangles;
    if (mVertexCount > 0)
    {
	    for (size_t i = 0; i < mVertexCount - 2; ++i)
	    {
            glm::vec4 vertex0 { mVertices[0], 1.0f };
            glm::vec4 vertex1 { mVertices[i + 1], 1.0f };
            glm::vec4 vertex2 { mVertices[i + 2], 1.0f };

            std::array<glm::vec4, 3> triangleVertices = { vertex0, vertex1, vertex2 };
            clippedTriangles.push_back(std::move(triangleVertices));
        }
    }

    return clippedTriangles;
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