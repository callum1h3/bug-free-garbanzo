#pragma once

#include "Renderer.h"
#include "glm/vec3.hpp"
#include "FastNoise/FastNoise.h"
#include "SafeQueue.h"

#include <map>


	static const int VOXEL_CHUNK_SIZE = 32;
	static const int VOXEL_CHUNK_HEIGHT = 512;
	static const int VOXEL_CHUNK_SUB_COUNT = VOXEL_CHUNK_HEIGHT / VOXEL_CHUNK_SIZE;
	static const int VOXEL_CHUNK_SUB_CUBED = VOXEL_CHUNK_SIZE * VOXEL_CHUNK_SIZE * VOXEL_CHUNK_SIZE;
	static const int VOXEL_CHUNK_SQUARED = VOXEL_CHUNK_SIZE * VOXEL_CHUNK_SIZE;
	static const int VOXEL_CHUNK_CUBED = VOXEL_CHUNK_HEIGHT * VOXEL_CHUNK_SQUARED;
	static const int VOXEL_SIZE_MINUS_ONE = VOXEL_CHUNK_SIZE - 1;
	static const int VOXEL_SQUARED_WERID_CALCULATION = VOXEL_SIZE_MINUS_ONE * VOXEL_CHUNK_SIZE;
	static const int VOXEL_CUBED_WERID_CALCULATION = VOXEL_SIZE_MINUS_ONE * VOXEL_CHUNK_SIZE * VOXEL_CHUNK_SIZE;

	static const glm::vec3 VOXEL_VERTICES[8] = {
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(1.0f, 0.0f, 1.0f),
		glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(0.0f, 1.0f, 1.0f),
	};

	static const int VOXEL_TRIANGLES[6][6] = {
		{0, 3, 1, 1, 3, 2},
		{5, 6, 4, 4, 6, 7},
		{3, 7, 2, 2, 7, 6},
		{1, 5, 0, 0, 5, 4},
		{4, 7, 0, 0, 7, 3},
		{1, 2, 5, 5, 2, 6}
	};

	static const glm::ivec2 neighbourChunks[4] = {
		// Back
		glm::ivec2(0, -1),
		// Front
		glm::ivec2(0,  1),
		// Left
		glm::ivec2(-1, 0),
		// Right
		glm::ivec2(1,  0)
	};

	struct Voxel;
	struct Chunk;

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
		unsigned int BUFFER_VBO_ID;
		unsigned int BUFFER_VAO_ID;
		unsigned int TRIANGLE_COUNT;

		bool IS_CURRENTLY_UPDATING;
		bool IS_OUTDATED;

		unsigned int CURRENT_UPDATE_ID;
		unsigned int LATEST_MESH_UPDATE_ID;

		glm::vec3 CHUNK_BUFFER_WORLD_POSITION;

		void Initialize();
		void Dispose();
		void Render(int sub_id);
	};

	class ChunkRenderer
	{
	public:
		bool IS_RENDERERING = false;
		Chunk* CHUNK;

		void Initialize();
		void Render();
		void Dispose();

		void Attach(Chunk* chunk);
		void UpdateVBO(std::vector<VoxelVertexInfo> vertices, int sub_id);

		ChunkBuffer* BUFFERS;
	};

	struct ChunkThreadInitInfo
	{
	public:
		glm::ivec2 CHUNK_POSITION;
		bool VALID_CHECK;
	};

	struct ChunkThreadMeshInfo
	{
	public:
		std::vector<VoxelVertexInfo> VERTICES;
		glm::ivec2 CHUNK_POSITION;
		bool VALID_CHECK;
		int SUB_ID;
		unsigned int UPDATE_ID;
	};

	struct Voxel
	{
	public:
		unsigned short ID;
		unsigned char  DATA;
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
		void FindRenderer();
		Voxel* GetVoxel(int access);

		void CalculateVisability();
		static std::vector<VoxelVertexInfo> CalculateChunkMesh(Chunk* chunk, int sub_id);
	private:

		bool CalculateBlockFaceVisabilityY(Voxel* voxel, int visability_index_source, int visability_index_connect, int access, int dir_coord, int edge_number, int inside_chunk_direction);
		bool CalculateBlockFaceVisability(Voxel* voxel, Chunk* bordering_chunk, int visability_index_source, int visability_index_connect, int access, int dir_coord, int edge_number, int outside_chunk_diretion, int inside_chunk_direction);
		unsigned char CalculateBlockVisability(Voxel* voxel, int access, int x, int y, int z);
		bool CanFaceRenderAgainst(Voxel* source_voxel, Voxel* voxel);
	};

	class VoxelManager
	{
	public:
		static unsigned int SHADER_CHUNK_POSITION_BINDING;
		static unsigned int SHADER_CHUNK_SUB_ID_BINDING;
		static Shader* SHADER_CHUNK;
		static bool VOXEL_CONFIG_FRUSTUM;
		static int CURRENT_TRIANGLE_COUNT;

		static Camera* RENDERER_CAMERA;
		static Frustum* RENDERER_VIEW_FRUSTUM;
		static Texture* CHUNK_TEXTURE;

		static void Initialize();
		static void Update();
		static void Dispose();

		static void SetTexturePack(std::string name);

		// CHUNK
		static Chunk* GetChunk(int x, int z);
		static Chunk* GetChunk(glm::ivec2 pos);

		static int CHUNK_RENDERERS_COUNT;
		static ChunkRenderer* CHUNK_RENDERERS;

		// Thread
		static SafeQueue<ChunkThreadInitInfo> CHUNK_INIT_THREAD_IN;
		static std::deque<ChunkThreadInitInfo> CHUNK_INIT_THREAD_OUT;

		static SafeQueue<ChunkThreadMeshInfo> CHUNK_MESH_THREAD_IN;
		static std::deque<ChunkThreadMeshInfo> CHUNK_MESH_THREAD_OUT;


	private:
		// THREAD
		static bool CHUNK_MESH_THREAD_ALIVE;
		static std::thread CHUNK_INIT_THREAD;
		static std::thread CHUNK_MESH_THREAD;

		static void InitializeThreads();
		static void LoopThreads();
		static void DisposeThreads();
		static void ChunkInitThread();
		static void ChunkMeshThread();

		// CHUNK
		static int CHUNK_RENDER_DISTANCE;
		static std::map<int, std::map<int, Chunk>> CHUNKS_MAP;

		static void InitializeRenderers();
		static void ChunkUpdateLoop();
		static void RendererLoop();
		static void DisposeRenderers();
	};
