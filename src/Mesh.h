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
	Mesh();
	void Load(const std::vector<glm::vec3>& vertices, const std::vector<glm::vec3>& normals, const std::vector<glm::vec2>& uvs,
			  const std::vector<Face>& faces);
	size_t FaceCount() const { return mFaces.size(); }
	const glm::vec3& GetVertex(size_t index) const { return mVertices[index]; }
	const glm::vec3& GetNormal(size_t index) const { return mNormals[index]; }
	const Face& GetFace(size_t index) const { return mFaces[index]; }
	
	const glm::vec3& GetRotation() const { return mRotation; }
	void SetRotation(const glm::vec3& rotation) { mRotation = rotation; }
	void AddRotation(const glm::vec3& rotation) { mRotation += rotation; }

	const glm::vec3& GetScale() const { return mScale; }
	void SetScale(const glm::vec3& scale) { mScale = scale; }
	void AddScale(const glm::vec3& scale) { mScale += scale; }

	const glm::vec3& GetTranslation() const { return mTranslation; }
	void SetTranslation(const glm::vec3& translation) { mTranslation = translation; }
	void AddTranslation(const glm::vec3& translation) { mTranslation += translation; }

private:
	std::vector<glm::vec3> mVertices;
	std::vector<glm::vec3> mNormals;
	std::vector<glm::vec2> mUvs;
	std::vector<Face> mFaces;
	glm::vec3 mRotation;
	glm::vec3 mScale;
	glm::vec3 mTranslation;
};

//------------------------------------------------------------------------------
std::unique_ptr<Mesh> CreateMeshFromOBJFile(const fs::path& filepath);