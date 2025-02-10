#include "Mesh.h"

// Includes
//------------------------------------------------------------------------------
// System
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>

//------------------------------------------------------------------------------
Mesh::Mesh()
	: mRotation(0.0f)
	, mScale(1.0f)
	, mTranslation(0.0f)
{ }

//------------------------------------------------------------------------------
void Mesh::Load(const std::vector<glm::vec3>& vertices, const std::vector<glm::vec3>& normals, const std::vector<glm::vec2>& uvs,                
                const std::vector<Face>& faces)
{
    mVertices = vertices;
	mNormals = normals;
	mUvs = uvs;
    mFaces = faces;
}

//------------------------------------------------------------------------------
std::unique_ptr<Mesh> CreateMeshFromOBJFile(const fs::path& filepath)
{
    std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();

    std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
    std::vector<Face> faces;

    std::ifstream file(filepath);
    if (!file)
    {
        std::cerr << "Error: Could not open file " << filepath << std::endl;
        return nullptr;
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#') continue; // Skip empty lines and comments

        if (line[0] == 'v' && line[1] == ' ') // Vertex
        {
            glm::vec3 vertex;
            if (sscanf_s(line.c_str(), "v %f %f %f", &vertex.x, &vertex.y, &vertex.z) == 3)
            {
                vertices.push_back(vertex);
            }
        }       
        else if (line[0] == 'v' && line[1] == 'n') // Vertex
        {
            glm::vec3 normal;
            if (sscanf_s(line.c_str(), "vn %f %f %f", &normal.x, &normal.y, &normal.z) == 3)
            {
                normals.push_back(normal);
            }
		}
		else if (line[0] == 'v' && line[1] == 't') // UV
		{
			glm::vec2 uv;
			if (sscanf_s(line.c_str(), "vt %f %f", &uv.x, &uv.y) == 2)
			{
				uvs.push_back(uv);
			}
		}
        else if (line[0] == 'f' && line[1] == ' ') // Face
        {
            Face face;
            if (sscanf_s(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d",
                &face.mVertexIndicies[0], &face.mTextureIndicies[0], &face.mNormalIndicies[0],
                &face.mVertexIndicies[1], &face.mTextureIndicies[1], &face.mNormalIndicies[1],
                &face.mVertexIndicies[2], &face.mTextureIndicies[2], &face.mNormalIndicies[2]) == 9)
            {
				// Convert to 0-based indices
                face.mVertexIndicies[0]--, face.mTextureIndicies[0]--, face.mNormalIndicies[0]--;
                face.mVertexIndicies[1]--, face.mTextureIndicies[1]--, face.mNormalIndicies[1]--;
                face.mVertexIndicies[2]--, face.mTextureIndicies[2]--, face.mNormalIndicies[2]--;
                
                faces.push_back(face);
            }
        }
    }

    mesh->Load(vertices, normals, uvs, faces);
    return mesh;
}