#include "voxelmanager.h"
#include "SystemHelper.h"
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"


bool VoxelManager::CHUNK_MESH_THREAD_ALIVE = false;
std::thread VoxelManager::CHUNK_INIT_THREAD;
std::thread VoxelManager::CHUNK_MESH_THREAD;

SafeQueue<ChunkThreadInitInfo> VoxelManager::CHUNK_INIT_THREAD_IN;
std::deque<ChunkThreadInitInfo> VoxelManager::CHUNK_INIT_THREAD_OUT;

SafeQueue<ChunkThreadMeshInfo> VoxelManager::CHUNK_MESH_THREAD_IN;
std::deque<ChunkThreadMeshInfo> VoxelManager::CHUNK_MESH_THREAD_OUT;

unsigned int VoxelManager::SHADER_CHUNK_SUB_ID_BINDING;
unsigned int VoxelManager::SHADER_CHUNK_POSITION_BINDING;
Shader* VoxelManager::SHADER_CHUNK;
bool VoxelManager::VOXEL_CONFIG_FRUSTUM = true;
int VoxelManager::CURRENT_TRIANGLE_COUNT = 0;

Camera* VoxelManager::RENDERER_CAMERA;
Frustum* VoxelManager::RENDERER_VIEW_FRUSTUM;
Texture* VoxelManager::CHUNK_TEXTURE;

std::map<int, std::map<int, Chunk>> VoxelManager::CHUNKS_MAP;
int VoxelManager::CHUNK_RENDER_DISTANCE = 4;
int VoxelManager::CHUNK_RENDERERS_COUNT = 0;

ChunkRenderer* VoxelManager::CHUNK_RENDERERS;

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
}

void VoxelManager::Render()
{
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

					Voxel* voxel = chunk->GetVoxel(access);
					voxel->ID = 0;
					voxel->VISABILITY = 0;

					if (perlin > 0.01f)
						voxel->ID = 1;
				}
			}
		}

		chunk->CalculateVisability();
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

void VoxelManager::ChunkUpdateLoop()
{
	glm::vec3 camera_position = RENDERER_CAMERA->GetPosition();
	glm::ivec2 camera_chunk_position = glm::ivec2(glm::round(camera_position.x / VOXEL_CHUNK_SIZE), glm::round(camera_position.z / VOXEL_CHUNK_SIZE));

	for (int x = -CHUNK_RENDER_DISTANCE; x < CHUNK_RENDER_DISTANCE; x++)
	{
		for (int z = -CHUNK_RENDER_DISTANCE; z < CHUNK_RENDER_DISTANCE; z++)
		{
			glm::ivec2 current_chunk_position = glm::ivec2(x, z) + camera_chunk_position;
			Chunk* chunk = GetChunk(current_chunk_position.x, current_chunk_position.y);

			if (!chunk->IS_VALID)
				chunk->Create(current_chunk_position);

			if (!chunk->IS_RENDERERING && chunk->IS_INITIALIZED)
				chunk->FindRenderer(camera_chunk_position);
		}
	}
}

Chunk* VoxelManager::GetChunk(int x, int z) { return &CHUNKS_MAP[x][z]; }
Chunk* VoxelManager::GetChunk(glm::ivec2 pos) { return GetChunk(pos.x, pos.y); }

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
