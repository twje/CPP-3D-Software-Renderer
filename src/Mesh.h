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
struct Face
{
	std::array<int32_t, 3> mVertexIndicies;
	std::array<int32_t, 3> mTextureIndicies;
	std::array<int32_t, 3> mNormalIndicies;
};

//------------------------------------------------------------------------------
class Mesh
{
public:
	void Load(const std::vector<glm::vec3>& vertices, const std::vector<glm::vec3>& normals, const std::vector<glm::vec2>& uvs,
			  const std::vector<Face>& faces);
	
	size_t FaceCount() const { return mFaces.size(); }
	const glm::vec3& GetVertex(size_t index) const { return mVertices[index]; }
	const glm::vec3& GetNormal(size_t index) const { return mNormals[index]; }
	const glm::vec2& GetUV(size_t index) const { return mUvs[index]; }
	const Face& GetFace(size_t index) const { return mFaces[index]; }
	
private:
	std::vector<glm::vec3> mVertices;
	std::vector<glm::vec3> mNormals;
	std::vector<glm::vec2> mUvs;
	std::vector<Face> mFaces;
};

//------------------------------------------------------------------------------
std::unique_ptr<Mesh> CreateMeshFromOBJFile(const fs::path& filepath);