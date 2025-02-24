#include "Clipping.h"

// Includes
//------------------------------------------------------------------------------
// System
#include <algorithm>

namespace {

}  // namespace

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
std::vector<Triangle> ClipWithFrustum(const std::array<Plane, 6>& planes, const Triangle& triangle)
{
    /*
        Sutherland-Hodgman polygon clipping algorithm
    */

    // Maximum number of vertices when clipping a triangle against a frustum
	constexpr size_t kMaxVertices = 10;

    // Start with the triangle's vertices
    std::array<glm::vec3, kMaxVertices> vertices {
		triangle.mVertices[0].mPoint,
        triangle.mVertices[1].mPoint,
        triangle.mVertices[2].mPoint,
    };
    size_t vertexCount = 3;

    // Clip the polygon against each plane.
    for (size_t i = 0; i < static_cast<size_t>(ClippingPlaneType::COUNT) && vertexCount > 0; ++i)
    {
        const Plane& plane = planes[i];
        std::array<glm::vec3, kMaxVertices> newVertices;
        size_t newCount = 0;

        for (size_t j = 0; j < vertexCount; ++j)
        {
            const glm::vec3& current = vertices[j];
            const glm::vec3& next = vertices[(j + 1) % vertexCount];

            const float currentDist = glm::dot(plane.mNormal, current - plane.mPoint);
            const float nextDist = glm::dot(plane.mNormal, next - plane.mPoint);

            // If the current vertex is inside, add it.
            if (currentDist >= 0.0f)
            {
                newVertices[newCount++] = current;
            }

            // If the edge crosses the plane, compute and add the intersection point.
            if (currentDist * nextDist < 0.0f)
            {
                const float t = currentDist / (currentDist - nextDist);
                newVertices[newCount++] = glm::mix(current, next, t);
            }
        }

        std::copy(newVertices.begin(), newVertices.begin() + newCount, vertices.begin());
        vertexCount = newCount;
    }

    // Triangulate the resulting polygon using a fan (if it has at least 3 vertices).
    std::vector<Triangle> clippedTriangles;
    if (vertexCount >= 3)
    {
		Triangle clippedTriangle;

        for (size_t i = 0; i < vertexCount - 2; i++)
        {
            clippedTriangle.mVertices[0].mPoint = glm::vec4(vertices[0], 1.0f);
            clippedTriangle.mVertices[1].mPoint = glm::vec4(vertices[i + 1], 1.0f);
            clippedTriangle.mVertices[2].mPoint = glm::vec4(vertices[i + 2], 1.0f);

			clippedTriangles.push_back(clippedTriangle);
        }
    }

    return clippedTriangles;
}