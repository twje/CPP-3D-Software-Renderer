#pragma once

// Includes
//------------------------------------------------------------------------------
// Application
#include "Trangle.h"

// Core
#include "Core/Graphics/Vector.h"

// System
#include <vector>

//------------------------------------------------------------------------------
extern const std::vector<Vector3f> kCubeVertices;
extern const std::vector<Face> kCubeFaces;

//------------------------------------------------------------------------------
class Mesh
{
public:
	void Load(const std::vector<Vector3f>& vertices, const std::vector<Face>& faces);	
	size_t FaceCount() const { return mFaces.size(); }
	const Vector3f& GetVertex(size_t index) const { return mVertices[index]; }
	const Face& GetFace(size_t index) const { return mFaces[index]; }
	const Vector3f& GetRotation() const { return mRotation; }	
	void AddRotation(const Vector3f& rotation) { mRotation += rotation; }

private:
	std::vector<Vector3f> mVertices;
	std::vector<Face> mFaces;
	Vector3f mRotation;
};