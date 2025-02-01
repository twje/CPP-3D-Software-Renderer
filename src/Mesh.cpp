#include "Mesh.h"

//------------------------------------------------------------------------------
const std::vector<Vector3f> kMeshVertices = {
	{ -1.0f, -1.0f, -1.0f }, // 1
	{ -1.0f,  1.0f, -1.0f }, // 2
	{  1.0f,  1.0f, -1.0f }, // 3
	{  1.0f, -1.0f, -1.0f }, // 4
	{  1.0f,  1.0f,  1.0f }, // 5
	{  1.0f, -1.0f,  1.0f }, // 6
	{ -1.0f,  1.0f,  1.0f }, // 7
	{ -1.0f, -1.0f,  1.0f }  // 8
};

// Clockwise winding order for front face
const std::vector<Face> kMeshFaces = {
    // front
    { 1, 2, 3 },
    { 1, 3, 4 },
    // right
    { 4, 3, 5 },
    { 4, 5, 6 },
    // back
    { 6, 5, 7 },
    { 6, 7, 8 },
    // left
    { 8, 7, 2 },
    { 8, 2, 1 },
    // top
    { 2, 7, 5 },
    { 2, 5, 3 },
    // bottom
    { 6, 8, 1 },
    { 6, 1, 4 }
};