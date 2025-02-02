#include "Mesh.h"

// Includes
//------------------------------------------------------------------------------
// System
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>

//------------------------------------------------------------------------------
void Mesh::Load(const std::vector<Vector3f>& vertices, const std::vector<Face>& faces)
{
    mVertices = vertices;
    mFaces = faces;
}

std::unique_ptr<Mesh> CreateMeshFromOBJFile(const fs::path& filepath)
{
    std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();

    std::vector<Vector3f> vertices;
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
            Vector3f vertex;
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


/*
//------------------------------------------------------------------------------
std::unique_ptr<Mesh> CreateMeshFromOBJFile(const fs::path& filepath)
{
	std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();
	
	std::vector<Vector3f> vertices;
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
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") // Vertex
        {
            Vector3f vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            vertices.push_back(vertex);
        }
        else if (prefix == "f") // Face
        {            
            int32_t indices[3];
            std::string vertexData;

            for (int i = 0; i < 3; i++)
            {
                if (!(iss >> vertexData))
                {
                    std::cerr << "Error: Face definition does not have exactly 3 vertices." << std::endl;
                    return nullptr;
                }

                std::istringstream vertexStream(vertexData);
                std::string index;
                std::getline(vertexStream, index, '/'); // Extract vertex index
                indices[i] = std::stoi(index) - 1;
            }

            Face face;
            face.a = indices[0];
            face.b = indices[1];
            face.c = indices[2];

			faces.push_back(face);
        }
    }

	mesh->Load(vertices, faces);

	return mesh;
}
*/