#pragma once

/*
* This header contains all global defines and constants 
for geometry importing, loading and manipulation.
*/

// Specifies what offset is globally applied to vertex indexes
// Example: if 0 -> index 0 points to vertex 0; if 1 -> index 1 points to vertex 0
#define VERTEX_INDEX_OFFSET	1

#define ASSIMP_PREPROCESS_FLAGS aiProcess_Triangulate | aiProcess_JoinIdenticalVertices | aiProcess_FlipUVs

const std::vector<const char*> c_supportedFormats = { ".obj", ".fbx"};