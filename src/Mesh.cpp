#include "Mesh.h"

// Includes
//------------------------------------------------------------------------------
// System
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>

//------------------------------------------------------------------------------
void Mesh::Load(const std::vector<glm::vec3>& vertices, const std::vector<Face>& faces)
{
    mVertices = vertices;
    mFaces = faces;
}

//------------------------------------------------------------------------------
std::unique_ptr<Mesh> CreateMeshFromOBJFile(const fs::path& filepath)
{
    std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();

    std::vector<glm::vec3> vertices;
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
        else if (line[0] == 'f' && line[1] == ' ') // Face
        {
            Face face;
            if (sscanf_s(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d",
                &face.a, &face.at, &face.an,
                &face.b, &face.bt, &face.bn,
                &face.c, &face.ct, &face.cn) == 9)
            {
				// Convert to 0-based indices
                face.a--, face.at--, face.an--;
                face.b--, face.bt--, face.bn--;
                face.c--, face.ct--, face.cn--;
                
                faces.push_back(face);
            }
        }
    }

    mesh->Load(vertices, faces);
    return mesh;
}