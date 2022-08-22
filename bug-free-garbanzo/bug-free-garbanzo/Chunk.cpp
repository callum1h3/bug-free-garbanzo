#include "Chunk.h"
#include "VoxelManager.h"

void Chunk::Create(glm::ivec2 chunk_pos)
{
	CHUNK_POSITION = chunk_pos;
	WORLD_POSITION = glm::vec2(chunk_pos.x * VOXEL_CHUNK_SIZE, chunk_pos.y * VOXEL_CHUNK_SIZE);
	IS_VALID = true;

	for (int i = 0; i < 4; i++)
	{
		glm::ivec2 dir = neighbourChunks[i];
		BORDERING_CHUNKS[i] = VoxelManager::GetChunk(CHUNK_POSITION + dir);
	}

	ChunkThreadInitInfo info;
	info.CHUNK_POSITION = chunk_pos;
	info.VALID_CHECK = true;

	VoxelManager::CHUNK_INIT_THREAD_IN.enqueue(info);
}

Voxel* Chunk::GetVoxel(int access)
{
	return &VOXELS[access];
}

void Chunk::FindRenderer(glm::ivec2 camera_position)
{
	for (int i = 0; i < VoxelManager::CHUNK_RENDERERS_COUNT; i++)
	{
		ChunkRenderer* renderer = &VoxelManager::CHUNK_RENDERERS[i];

		bool should_attach = false;
		if (!renderer->IS_RENDERERING)
			should_attach = true;
		else
		{
			glm::ivec2 c_position = renderer->CHUNK->CHUNK_POSITION;
			int distance1 = glm::abs(camera_position.x - c_position.x);
			int distance2 = glm::abs(camera_position.y - c_position.y);

			if (distance1 > VoxelManager::CHUNK_RENDER_DISTANCE || distance2 > VoxelManager::CHUNK_RENDER_DISTANCE)
				should_attach = true;
		}

		if (should_attach)
		{
			renderer->Attach(this);

			for (int i = 0; i < VOXEL_CHUNK_SUB_COUNT; i++)
				OnUpdate(i, false, false);

			return;
		}
	}
}

void Chunk::OnUpdate(int sub_id, bool is_first = false, bool is_source = true)
{
	if (is_source)
		UpdateBordering(sub_id, true);

	if (!IS_INITIALIZED)
		return;

	if (!IS_RENDERERING)
		return;

	ChunkBuffer* buffer = &RENDERER->BUFFERS[sub_id];

	ChunkThreadMeshInfo info;
	info.SUB_ID = sub_id;
	info.CHUNK_POSITION = CHUNK_POSITION;
	info.VALID_CHECK = true;
	info.UPDATE_ID = buffer->CURRENT_UPDATE_ID;
	buffer->CURRENT_UPDATE_ID += 1;

	VoxelManager::CHUNK_MESH_THREAD_IN.enqueue(info);
}

void Chunk::UpdateBordering(int sub_id, bool should_update_y = true)
{
	if (should_update_y)
	{
		if (sub_id != VOXEL_CHUNK_SUB_COUNT - 1)
			OnUpdate(sub_id + 1, false, false);

		if (sub_id != 0)
			OnUpdate(sub_id - 1, false, false);
	}

	for (int i = 0; i < 4; i++)
		BORDERING_CHUNKS[i]->OnUpdate(sub_id, false, false);
}

std::vector<VoxelVertexInfo> Chunk::CalculateChunkMesh(Chunk* chunk, int sub_id)
{
	std::vector<VoxelVertexInfo> vertices;
	int access = VOXEL_CHUNK_SUB_CUBED * sub_id;

	for (int y = 0; y < VOXEL_CHUNK_SIZE; y++)
	{
		int by = y + (VOXEL_CHUNK_SIZE * sub_id);
		for (int x = 0; x < VOXEL_CHUNK_SIZE; x++)
		{
			for (int z = 0; z < VOXEL_CHUNK_SIZE; z++, access++)
			{
				Voxel* voxel = chunk->GetVoxel(access);

				if (voxel->ID == 0)
					continue;

				unsigned char visability = voxel->VISABILITY;

				for (int p = 0; p < 6; p++)
				{

					if (((visability >> p) & 1) == 1)
					{
						for (int i = 0; i < 6; i++)
						{
							int triangleIndex = VOXEL_TRIANGLES[p][i];
							glm::vec3 vertex_pos = VOXEL_VERTICES[triangleIndex] + glm::vec3(x, y, z);

							unsigned short compressed_x = (unsigned short)glm::round(vertex_pos.x * 256);
							unsigned short compressed_y = (unsigned short)glm::round(vertex_pos.y * 256);
							unsigned short compressed_z = (unsigned short)glm::round(vertex_pos.z * 256);

							GLubyte vertexInfo = (GLubyte)((i << 4) | p);

							VoxelVertexInfo vertex_info;
							vertex_info.VERTEX_1 = compressed_x | (compressed_y << 16);
							vertex_info.VERTEX_2 = compressed_z | (0 << 16);
							vertex_info.VERTEX_3 = vertexInfo | (0 << 8) | (0 << 16) | (0 << 24);

							vertices.push_back(vertex_info);
						}
					}
				}
			}
		}
	}

	return vertices;
}

void Chunk::CalculateVisability()
{
	int access = 0;
	for (int by = 0; by < VOXEL_CHUNK_HEIGHT; by++)
	{
		for (int bx = 0; bx < VOXEL_CHUNK_SIZE; bx++)
		{
			for (int bz = 0; bz < VOXEL_CHUNK_SIZE; bz++, access++)
			{
				Voxel* voxel = GetVoxel(access);

				CalculateBlockVisability(voxel, access, bx, by, bz);
			}
		}
	}
}

bool Chunk::CanFaceRenderAgainst(Voxel* source_voxel, Voxel* voxel)
{
	if (voxel->ID == 0)
		return true;

	return false;
}

void Chunk::CalculateBlockFaceVisabilityY(Voxel* voxel, int visability_index_source, int visability_index_connect, int access, int dir_coord, int edge_number, int inside_chunk_direction)
{
	if (dir_coord != edge_number)
	{
		Voxel* connecting_voxel = GetVoxel(access + inside_chunk_direction);

		connecting_voxel->VISABILITY |= CanFaceRenderAgainst(connecting_voxel, voxel) << visability_index_connect;
		voxel->VISABILITY |= CanFaceRenderAgainst(voxel, connecting_voxel) << visability_index_source;
	}

	return;
}

void Chunk::CalculateBlockFaceVisability(Voxel* voxel, Chunk* bordering_chunk, int visability_index_source, int visability_index_connect, int access, int dir_coord, int edge_number, int outside_chunk_diretion, int inside_chunk_direction)
{
	Voxel* connecting_voxel = nullptr;
	bool has_voxel = false;

	if (dir_coord == edge_number)
	{
		if ((bordering_chunk->IS_VALID && bordering_chunk->IS_INITIALIZED))
		{
			connecting_voxel = bordering_chunk->GetVoxel(access + outside_chunk_diretion);
			has_voxel = true;
		}
	}
	else
	{
		connecting_voxel = GetVoxel(access + inside_chunk_direction);
		has_voxel = true;
	}

	if (has_voxel)
	{
		connecting_voxel->VISABILITY |= CanFaceRenderAgainst(connecting_voxel, voxel) << visability_index_connect;
		voxel->VISABILITY |= CanFaceRenderAgainst(voxel, connecting_voxel) << visability_index_source;
	}
}

void Chunk::CalculateBlockVisability(Voxel* voxel, int access, int x, int y, int z)
{
	// Back
	CalculateBlockFaceVisability(voxel, BORDERING_CHUNKS[0], 0, 1, access, z, 0, VOXEL_SIZE_MINUS_ONE, -1);
	// Front
	CalculateBlockFaceVisability(voxel, BORDERING_CHUNKS[1], 1, 0, access, z, VOXEL_SIZE_MINUS_ONE, -VOXEL_SIZE_MINUS_ONE, 1);

	// Up
	CalculateBlockFaceVisabilityY(voxel, 2, 3, access, y, VOXEL_CHUNK_HEIGHT - 1, VOXEL_CHUNK_SQUARED);
	// Down
	CalculateBlockFaceVisabilityY(voxel, 3, 2, access, y, 0, -VOXEL_CHUNK_SQUARED);

	// Left
	CalculateBlockFaceVisability(voxel, BORDERING_CHUNKS[2], 4, 5, access, x, 0, VOXEL_SQUARED_WERID_CALCULATION, -VOXEL_CHUNK_SIZE);
	// Right
	CalculateBlockFaceVisability(voxel, BORDERING_CHUNKS[3], 5, 4, access, x, VOXEL_SIZE_MINUS_ONE, -VOXEL_SQUARED_WERID_CALCULATION, VOXEL_CHUNK_SIZE);
}