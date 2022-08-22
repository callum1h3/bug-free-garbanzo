#pragma once

#include "VoxelHelper.h"
#include "ChunkRenderer.h"
#include <vector>
#include <map>

struct Voxel
{
public:
	unsigned short ID;
	unsigned char  VISABILITY;
};

class Chunk
{
public:
	glm::ivec2 CHUNK_POSITION;
	glm::vec2  WORLD_POSITION;

	std::map<unsigned int, Chunk*> BORDERING_CHUNKS;
	ChunkRenderer* RENDERER;
	Voxel* VOXELS;

	bool IS_VALID = false;
	bool IS_INITIALIZED = false;
	bool IS_RENDERERING = false;

	void Create(glm::ivec2 chunk_pos);
	void OnUpdate(int sub_id, bool is_first, bool is_source);
	void UpdateBordering(int sub_id, bool should_update_y);
	void FindRenderer(glm::ivec2 camera_position);
	void CalculateVisability();

	Voxel* GetVoxel(int access);
	static std::vector<VoxelVertexInfo> CalculateChunkMesh(Chunk* chunk, int sub_id);
private:

	void CalculateBlockFaceVisabilityY(Voxel* voxel, int visability_index_source, int visability_index_connect, int access, int dir_coord, int edge_number, int inside_chunk_direction);
	void CalculateBlockFaceVisability(Voxel* voxel, Chunk* bordering_chunk, int visability_index_source, int visability_index_connect, int access, int dir_coord, int edge_number, int outside_chunk_diretion, int inside_chunk_direction);
	void CalculateBlockVisability(Voxel* voxel, int access, int x, int y, int z);
	bool CanFaceRenderAgainst(Voxel* source_voxel, Voxel* voxel);
};