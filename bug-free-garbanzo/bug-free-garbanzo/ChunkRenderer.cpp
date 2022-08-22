#include "ChunkRenderer.h"
#include "VoxelManager.h"

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
	if (this->IS_RENDERERING)
	{
		this->CHUNK->IS_RENDERERING = false;
		this->CHUNK->RENDERER = nullptr;

		for (int i = 0; i < VOXEL_CHUNK_SUB_COUNT; i++)
			BUFFERS[i].Reset();
	}

	chunk->IS_RENDERERING = true;
	chunk->RENDERER = this;

	this->CHUNK = chunk;
	this->IS_RENDERERING = true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////// CHUNK BUFFER //////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

void ChunkBuffer::Reset()
{
	TRIANGLE_COUNT = 0;

	glBindBuffer(GL_ARRAY_BUFFER, BUFFER_VBO_ID);
	glBufferData(GL_ARRAY_BUFFER, 0, {}, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
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