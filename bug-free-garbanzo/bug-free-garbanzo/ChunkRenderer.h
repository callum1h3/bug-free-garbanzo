#pragma once

#include "glm/glm.hpp"
#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <vector>

class Chunk;

struct VoxelVertexInfo
{
public:
	unsigned int VERTEX_1;
	unsigned int VERTEX_2;
	unsigned int VERTEX_3;
};

struct ChunkBuffer
{
public:
	glm::vec3 CHUNK_BUFFER_WORLD_POSITION;

	unsigned int BUFFER_VBO_ID;
	unsigned int BUFFER_VAO_ID;
	unsigned int TRIANGLE_COUNT;

	unsigned int CURRENT_UPDATE_ID;
	unsigned int LATEST_MESH_UPDATE_ID;

	bool IS_CURRENTLY_UPDATING;
	bool IS_OUTDATED;

	void Initialize();
	void Dispose();
	void Render(int sub_id);
	void Reset();
};

class ChunkRenderer
{
public:
	bool IS_RENDERERING = false;
	ChunkBuffer* BUFFERS;
	Chunk* CHUNK;

	void Initialize();
	void Render();
	void Dispose();

	void Attach(Chunk* chunk);
	void UpdateVBO(std::vector<VoxelVertexInfo> vertices, int sub_id);
};