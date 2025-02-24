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

std::vector<Triangle> ClipWithFrustum(const std::array<Plane, 6>& planes, const Triangle& triangle)
{
    // Maximum number of vertices when clipping a triangle against a frustum
    constexpr size_t kMaxVertices = 10;

    // Start with the triangle's vertices.
    std::array<Vertex, kMaxVertices> vertices{};
    vertices[0] = triangle.mVertices[0];
    vertices[1] = triangle.mVertices[1];
    vertices[2] = triangle.mVertices[2];
    size_t vertexCount = 3;

    // Clip the polygon against each plane.
    for (const auto& plane : planes)
    {
        if (vertexCount == 0)
        {
            break;
        }

        std::array<Vertex, kMaxVertices> newVertices { };
        size_t newCount = 0;

        // Lambda to compute the signed distance from a vertex to the plane.
        auto distance = [&plane](const Vertex& v) -> float {
            return glm::dot(plane.mNormal, glm::vec3(v.mPoint) - plane.mPoint);
        };

        for (size_t j = 0; j < vertexCount; ++j)
        {
            const Vertex& current = vertices[j];
            const Vertex& next = vertices[(j + 1) % vertexCount];

            const float currentDist = distance(current);
            const float nextDist = distance(next);

            // If the current vertex is inside the plane, add it.
            if (currentDist >= 0.0f)
            {
                newVertices[newCount++] = current;
            }

            // If the edge crosses the plane, compute and add the intersection.
            if (currentDist * nextDist < 0.0f)
            {
                const float t = currentDist / (currentDist - nextDist);
                
                Vertex intersection;
                intersection.mPoint = glm::mix(current.mPoint, next.mPoint, t);
                intersection.mUV = glm::mix(current.mUV, next.mUV, t);
                newVertices[newCount++] = intersection;
            }
        }

        // Update vertices with the clipped result.
        std::copy(newVertices.begin(), newVertices.begin() + newCount, vertices.begin());
        vertexCount = newCount;
    }

    // Triangulate the resulting polygon using a triangle fan.
    std::vector<Triangle> clippedTriangles;
    if (vertexCount >= 3)
    {
        clippedTriangles.reserve(vertexCount - 2);
        for (size_t i = 0; i < vertexCount - 2; ++i)
        {
            Triangle clippedTriangle;
            clippedTriangle.mVertices[0] = vertices[0];
            clippedTriangle.mVertices[1] = vertices[i + 1];
            clippedTriangle.mVertices[2] = vertices[i + 2];
            clippedTriangles.push_back(clippedTriangle);
        }
    }

    return clippedTriangles;
}