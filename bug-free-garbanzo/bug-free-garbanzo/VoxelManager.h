#pragma once

#include "Renderer.h"
#include "Chunk.h"
#include "VoxelHelper.h"
#include "FastNoise/FastNoise.h"
#include "SafeQueue.h"

#include <map>

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
		static int CHUNK_RENDER_DISTANCE;
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
		static std::map<int, std::map<int, Chunk>> CHUNKS_MAP;

		static void InitializeRenderers();
		static void ChunkUpdateLoop();
		static void RendererLoop();
		static void DisposeRenderers();
	};
