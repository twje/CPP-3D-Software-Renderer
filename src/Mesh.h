#pragma once

// Includes
//------------------------------------------------------------------------------
// Application
#include "Trangle.h"

// Third Party
#include <glm/glm.hpp>

// System
#include <filesystem>
#include <vector>
#include <memory>

// Type Alias
//------------------------------------------------------------------------------
namespace fs = std::filesystem;

//------------------------------------------------------------------------------
class Mesh
{
public:
	void Load(const std::vector<glm::vec3>& vertices, const std::vector<Face>& faces);
	size_t FaceCount() const { return mFaces.size(); }
	const glm::vec3& GetVertex(size_t index) const { return mVertices[index]; }
	const Face& GetFace(size_t index) const { return mFaces[index]; }
	const glm::vec3& GetRotation() const { return mRotation; }
	void AddRotation(const glm::vec3& rotation) { mRotation += rotation; }

private:
	std::vector<glm::vec3> mVertices;
	std::vector<Face> mFaces;
	glm::vec3 mRotation;
};

//------------------------------------------------------------------------------
std::unique_ptr<Mesh> CreateMeshFromOBJFile(const fs::path& filepath);