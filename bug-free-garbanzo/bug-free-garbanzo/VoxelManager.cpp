#include "voxelmanager.h"
#include "SystemHelper.h"
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"


	// BASIC FUNCTIONS
	unsigned int VoxelManager::SHADER_CHUNK_SUB_ID_BINDING;
	unsigned int VoxelManager::SHADER_CHUNK_POSITION_BINDING;
	Shader* VoxelManager::SHADER_CHUNK;
	bool VoxelManager::VOXEL_CONFIG_FRUSTUM = true;
	int VoxelManager::CURRENT_TRIANGLE_COUNT = 0;

	Camera* VoxelManager::RENDERER_CAMERA;
	Frustum* VoxelManager::RENDERER_VIEW_FRUSTUM;

	Texture* VoxelManager::CHUNK_TEXTURE;

	void VoxelManager::Initialize()
	{
		RENDERER_CAMERA = Renderer::GetCamera();
		RENDERER_VIEW_FRUSTUM = RENDERER_CAMERA->GetViewFrustum();

		SHADER_CHUNK = ShaderManager::GetShader("chunkshader");
		SHADER_CHUNK_POSITION_BINDING = glGetUniformLocation(SHADER_CHUNK->shader_main_program, "chunk_position");
		SHADER_CHUNK_SUB_ID_BINDING = glGetUniformLocation(SHADER_CHUNK->shader_main_program, "chunk_sub_id");

		stbi_set_flip_vertically_on_load(true);

		CHUNK_TEXTURE = Renderer::CreateTexture();
		CHUNK_TEXTURE->Initialize(GL_TEXTURE_2D_ARRAY);
		CHUNK_TEXTURE->SetPointFilter();

		SetTexturePack("g");

		InitializeRenderers();
		InitializeThreads();
	}

	void VoxelManager::Update()
	{
		LoopThreads();
		ChunkUpdateLoop();

		RendererLoop();
	}

	void VoxelManager::Dispose()
	{
		DisposeThreads();
		DisposeRenderers();
	}

	void VoxelManager::SetTexturePack(std::string name)
	{
		int image_size;
		int nrChannels;

		std::string path = SystemHelper::PATH_DIRECTORY + "texturepacks/vanilla/blocks/grass_side.png";

		std::vector<unsigned char*> texture_byte;
		unsigned char* data = stbi_load(path.c_str(), &image_size, &image_size, &nrChannels, 0);

		std::cout << nrChannels;
		for (int x = 0; x < sizeof(data) / sizeof(unsigned char); x++)
			texture_byte.push_back(&data[x]);

		unsigned char* a = texture_byte[0];

		CHUNK_TEXTURE->SetTextureArray(a, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE);

	}

	// THREAD LOGIC
	bool VoxelManager::CHUNK_MESH_THREAD_ALIVE = false;
	std::thread VoxelManager::CHUNK_INIT_THREAD;
	std::thread VoxelManager::CHUNK_MESH_THREAD;

	SafeQueue<ChunkThreadInitInfo> VoxelManager::CHUNK_INIT_THREAD_IN;
	std::deque<ChunkThreadInitInfo> VoxelManager::CHUNK_INIT_THREAD_OUT;

	SafeQueue<ChunkThreadMeshInfo> VoxelManager::CHUNK_MESH_THREAD_IN;
	std::deque<ChunkThreadMeshInfo> VoxelManager::CHUNK_MESH_THREAD_OUT;

	void VoxelManager::InitializeThreads()
	{
		CHUNK_MESH_THREAD_ALIVE = true;
		CHUNK_INIT_THREAD = std::thread(ChunkInitThread);
		CHUNK_MESH_THREAD = std::thread(ChunkMeshThread);
	}

	void VoxelManager::LoopThreads()
	{
		if (CHUNK_INIT_THREAD_OUT.size() > 0)
		{
			for (int i = 0; i < 100; i++)
			{
				if (CHUNK_INIT_THREAD_OUT.size() == 0)
					break;

				ChunkThreadInitInfo outInfo = CHUNK_INIT_THREAD_OUT[0];
				Chunk* chunk = GetChunk(outInfo.CHUNK_POSITION);

				if (chunk->IS_VALID)
				{
					chunk->IS_INITIALIZED = true;

					for (int i = 0; i < VOXEL_CHUNK_SUB_COUNT; i++)
						chunk->UpdateBordering(i, false);
				}

				CHUNK_INIT_THREAD_OUT.pop_front();
			}
		}

		if (CHUNK_MESH_THREAD_OUT.size() > 0)
		{
			for (int i = 0; i < 100; i++)
			{
				if (CHUNK_MESH_THREAD_OUT.size() == 0)
					break;
				
				ChunkThreadMeshInfo outInfo = CHUNK_MESH_THREAD_OUT[0];
				Chunk* chunk = GetChunk(outInfo.CHUNK_POSITION);

				if (chunk->IS_RENDERERING)
				{
					ChunkBuffer buffer = chunk->RENDERER->BUFFERS[outInfo.SUB_ID];
					if (!(buffer.LATEST_MESH_UPDATE_ID > outInfo.UPDATE_ID))
					{
						chunk->RENDERER->UpdateVBO(outInfo.VERTICES, outInfo.SUB_ID);
						buffer.LATEST_MESH_UPDATE_ID = outInfo.UPDATE_ID;
					}
				}

				CHUNK_MESH_THREAD_OUT.pop_front();
			}
		}
	}

	void VoxelManager::DisposeThreads()
	{
		CHUNK_MESH_THREAD_ALIVE = false;

		ChunkThreadInitInfo init_info;
		init_info.VALID_CHECK = false;
		CHUNK_INIT_THREAD_IN.enqueue(init_info);

		CHUNK_INIT_THREAD.join();

		ChunkThreadMeshInfo mesh_info;
		mesh_info.VALID_CHECK = false;
		CHUNK_MESH_THREAD_IN.enqueue(mesh_info);

		CHUNK_MESH_THREAD.join();
	}

	void VoxelManager::ChunkInitThread()
	{
		while (CHUNK_MESH_THREAD_ALIVE)
		{
			ChunkThreadInitInfo info = CHUNK_INIT_THREAD_IN.dequeue();

			if (!info.VALID_CHECK)
				continue;

			Chunk* chunk = GetChunk(info.CHUNK_POSITION);
			chunk->VOXELS = new Voxel[VOXEL_CHUNK_CUBED];

			auto simplex = FastNoise::New<FastNoise::OpenSimplex2>();
			auto fractal = FastNoise::New<FastNoise::FractalFBm>();

			fractal->SetSource(simplex);
			fractal->SetOctaveCount(2);
			fractal->SetGain(0.1f);
			fractal->SetLacunarity(10.0f);

			int access = 0;
			for (int by = 0; by < VOXEL_CHUNK_HEIGHT; by++)
			{
				for (int bx = 0; bx < VOXEL_CHUNK_SIZE; bx++)
				{
					int wx = bx + chunk->WORLD_POSITION.x;
					for (int bz = 0; bz < VOXEL_CHUNK_SIZE; bz++, access++)
					{
						int wz = bz + chunk->WORLD_POSITION.y;
						float perlin = fractal->GenSingle3D(wx * 0.01f, by * 0.01f, wz * 0.01f, 3000);
						float current_height = by;

						chunk->VOXELS[access].ID = 0;
						chunk->VOXELS[access].DATA = 0;

						if (perlin > 0.01f)
							chunk->VOXELS[access].ID = 1;
					}
				}
			}

			//chunk->CalculateVisability();
			CHUNK_INIT_THREAD_OUT.push_back(info);
		}

		std::cout << "THREAD CHUNK INIT DISPOSED\n";
	}

	void VoxelManager::ChunkMeshThread()
	{
		while (CHUNK_MESH_THREAD_ALIVE)
		{
			ChunkThreadMeshInfo info = CHUNK_MESH_THREAD_IN.dequeue();

			if (!info.VALID_CHECK)
				continue;

			Chunk* chunk = GetChunk(info.CHUNK_POSITION);
			if (!chunk->IS_INITIALIZED)
				return;

			std::vector<VoxelVertexInfo> vertices = chunk->CalculateChunkMesh(chunk, info.SUB_ID);
			info.VERTICES = vertices;

			CHUNK_MESH_THREAD_OUT.push_back(info);
		}

		std::cout << "THREAD CHUNK MESH DISPOSED\n";
	}

	// CHUNK MANAGERS
	std::map<int, std::map<int, Chunk>> VoxelManager::CHUNKS_MAP;
	int VoxelManager::CHUNK_RENDER_DISTANCE = 4;
	int VoxelManager::CHUNK_RENDERERS_COUNT = 0;

	void VoxelManager::ChunkUpdateLoop()
	{
		for (int x = -CHUNK_RENDER_DISTANCE; x < CHUNK_RENDER_DISTANCE; x++)
		{
			for (int z = -CHUNK_RENDER_DISTANCE; z < CHUNK_RENDER_DISTANCE; z++)
			{
				glm::ivec2 current_chunk_position = glm::ivec2(x, z);

				Chunk* chunk = GetChunk(x, z);

				if (!chunk->IS_VALID)
					chunk->Create(current_chunk_position);

				if (!chunk->IS_RENDERERING && chunk->IS_INITIALIZED)
					chunk->FindRenderer();
			}
		}
	}

	Chunk* VoxelManager::GetChunk(int x, int z) { return &CHUNKS_MAP[x][z]; }
	Chunk* VoxelManager::GetChunk(glm::ivec2 pos) { return GetChunk(pos.x, pos.y); }

	// CHUNK CLASS
	ChunkRenderer* VoxelManager::CHUNK_RENDERERS;

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

	void Chunk::FindRenderer()
	{
		for (int i = 0; i < VoxelManager::CHUNK_RENDERERS_COUNT; i++)
		{
			ChunkRenderer* renderer = &VoxelManager::CHUNK_RENDERERS[i];

			if (!renderer->IS_RENDERERING)
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

					unsigned char visability = chunk->CalculateBlockVisability(voxel, access, x, by, z);

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

	bool Chunk::CalculateBlockFaceVisabilityY(Voxel* voxel, int visability_index_source, int visability_index_connect, int access, int dir_coord, int edge_number, int inside_chunk_direction)
	{
		if (dir_coord != edge_number)
		{
			Voxel* connecting_voxel = GetVoxel(access + inside_chunk_direction);

			return CanFaceRenderAgainst(voxel, connecting_voxel);
		}

		return true;
	}

	bool Chunk::CalculateBlockFaceVisability(Voxel* voxel, Chunk* bordering_chunk, int visability_index_source, int visability_index_connect, int access, int dir_coord, int edge_number, int outside_chunk_diretion, int inside_chunk_direction)
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
			return CanFaceRenderAgainst(voxel, connecting_voxel);
		return false;
	}

	unsigned char Chunk::CalculateBlockVisability(Voxel* voxel, int access, int x, int y, int z)
	{
		unsigned char visability = 0;
		// Back
		visability |= CalculateBlockFaceVisability(voxel, BORDERING_CHUNKS[0], 0, 1, access, z, 0, VOXEL_SIZE_MINUS_ONE, -1) << 0;
		// Front
		visability |= CalculateBlockFaceVisability(voxel, BORDERING_CHUNKS[1], 1, 0, access, z, VOXEL_SIZE_MINUS_ONE, -VOXEL_SIZE_MINUS_ONE, 1) << 1;

		// Up
		visability |= CalculateBlockFaceVisabilityY(voxel, 2, 3, access, y, VOXEL_CHUNK_HEIGHT - 1, VOXEL_CHUNK_SQUARED) << 2;
		// Down
		visability |= CalculateBlockFaceVisabilityY(voxel, 3, 2, access, y, 0, -VOXEL_CHUNK_SQUARED) << 3;

		// Left
		visability |= CalculateBlockFaceVisability(voxel, BORDERING_CHUNKS[2], 4, 5, access, x, 0, VOXEL_SQUARED_WERID_CALCULATION, -VOXEL_CHUNK_SIZE) << 4;
		// Right
		visability |= CalculateBlockFaceVisability(voxel, BORDERING_CHUNKS[3], 5, 4, access, x, VOXEL_SIZE_MINUS_ONE, -VOXEL_SQUARED_WERID_CALCULATION, VOXEL_CHUNK_SIZE) << 5;

		return visability;
	}

	// RENDERER CLASS
	void VoxelManager::InitializeRenderers()
	{
		int render_double = CHUNK_RENDER_DISTANCE * 2;
		CHUNK_RENDERERS_COUNT = render_double * render_double;
		CHUNK_RENDERERS = new ChunkRenderer[CHUNK_RENDERERS_COUNT];

		for (int i = 0; i < CHUNK_RENDERERS_COUNT; i++)
			CHUNK_RENDERERS[i].Initialize();
	}

	void VoxelManager::RendererLoop()
	{
		CURRENT_TRIANGLE_COUNT = 0;

		CHUNK_TEXTURE->Use();
		SHADER_CHUNK->Use();
		for (int i = 0; i < CHUNK_RENDERERS_COUNT; i++)
		{
			ChunkRenderer* renderer = &CHUNK_RENDERERS[i];
			if (!renderer->IS_RENDERERING)
				continue;

			Chunk* chunk = renderer->CHUNK;

			glm::vec2 world_pos = chunk->WORLD_POSITION;
			glm::vec3 pos = glm::vec3(world_pos.x, 0, world_pos.y);

			bool is_on_screen = VoxelManager::RENDERER_VIEW_FRUSTUM->AABBisOnFrustum(pos, glm::vec3(VOXEL_CHUNK_SIZE, VOXEL_CHUNK_HEIGHT, VOXEL_CHUNK_SIZE));

			if (!is_on_screen)
				continue;

			glUniform2f(VoxelManager::SHADER_CHUNK_POSITION_BINDING, (GLfloat)chunk->CHUNK_POSITION.x, (GLfloat)chunk->CHUNK_POSITION.y);
			for (int i = 0; i < VOXEL_CHUNK_SUB_COUNT; i++)
			{
				glm::vec3 pos1 = glm::vec3(world_pos.x, i * 32, world_pos.y);
				bool is_on_screen1 = VoxelManager::RENDERER_VIEW_FRUSTUM->IsAABBVisable(pos1, 32);

				if (!is_on_screen1)
					continue;

				renderer->BUFFERS[i].Render(i);

			}
		}
	}

	void VoxelManager::DisposeRenderers()
	{
		for (int i = 0; i < CHUNK_RENDERERS_COUNT; i++)
			CHUNK_RENDERERS[i].Dispose();
		delete[] CHUNK_RENDERERS;
	}

	void ChunkRenderer::Initialize()
	{
		IS_RENDERERING = false;
		BUFFERS = new ChunkBuffer[VOXEL_CHUNK_SUB_COUNT];

		for (int i = 0; i < VOXEL_CHUNK_SUB_COUNT; i++)
			BUFFERS[i].Initialize();
	}

	void ChunkRenderer::Dispose()
	{
		for (int i = 0; i < VOXEL_CHUNK_SUB_COUNT; i++)
			BUFFERS[i].Dispose();

		delete[] BUFFERS;
	}

	void ChunkRenderer::Render()
	{
		if (!IS_RENDERERING)
			return;

		glUniform2f(VoxelManager::SHADER_CHUNK_POSITION_BINDING, (GLfloat)CHUNK->CHUNK_POSITION.x, (GLfloat)CHUNK->CHUNK_POSITION.y);
		for (int i = 0; i < VOXEL_CHUNK_SUB_COUNT; i++)
		{
			glm::vec3 pos = glm::vec3(CHUNK->CHUNK_POSITION.x * 32, i * 32, CHUNK->CHUNK_POSITION.y * 32);
			bool is_on_screen = VoxelManager::RENDERER_VIEW_FRUSTUM->AABBisOnFrustum(pos, glm::vec3(32, 512, 32));

			if (is_on_screen);
			{
				BUFFERS[i].Render(i);
			}
		}
	}

	void ChunkRenderer::UpdateVBO(std::vector<VoxelVertexInfo> vertices, int sub_id)
	{
		ChunkBuffer* buffer = &BUFFERS[sub_id];
		unsigned int buffer_id = buffer->BUFFER_VBO_ID;
		buffer->TRIANGLE_COUNT = std::size(vertices);

		glBindBuffer(GL_ARRAY_BUFFER, buffer_id);
		glBufferData(GL_ARRAY_BUFFER, buffer->TRIANGLE_COUNT * sizeof(vertices[0]), &vertices[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		vertices.clear();
	}

	void ChunkRenderer::Attach(Chunk* chunk)
	{
		chunk->IS_RENDERERING = true;
		chunk->RENDERER = this;

		this->CHUNK = chunk;
		this->IS_RENDERERING = true;
	}

	// CHUNK BUFFER
	void ChunkBuffer::Initialize()
	{
		CURRENT_UPDATE_ID = 1;
		LATEST_MESH_UPDATE_ID = 0;

		// VBO VAO BUFFER INITIALIZE
		glGenVertexArrays(1, &BUFFER_VAO_ID);
		glGenBuffers(1, &BUFFER_VBO_ID);

		glBindVertexArray(BUFFER_VAO_ID);

		glBindBuffer(GL_ARRAY_BUFFER, BUFFER_VBO_ID);
		glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);

		glVertexAttribIPointer(0, 3, GL_UNSIGNED_INT, 3 * sizeof(unsigned int), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		IS_CURRENTLY_UPDATING = false;
		IS_OUTDATED = false;
		TRIANGLE_COUNT = 0;
	}

	void ChunkBuffer::Dispose()
	{
		glDeleteBuffers(1, &BUFFER_VAO_ID);
		glDeleteBuffers(1, &BUFFER_VBO_ID);
	}

	void ChunkBuffer::Render(int sub_id)
	{
		if (TRIANGLE_COUNT < 1)
			return;

		VoxelManager::CURRENT_TRIANGLE_COUNT += TRIANGLE_COUNT;

		glUniform1i(VoxelManager::SHADER_CHUNK_SUB_ID_BINDING, sub_id);
		glBindVertexArray(BUFFER_VAO_ID);
		glDrawArrays(GL_TRIANGLES, 0, TRIANGLE_COUNT);
	}
