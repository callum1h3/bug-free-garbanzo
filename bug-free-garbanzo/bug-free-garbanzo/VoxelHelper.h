#pragma once

#include "glm/glm.hpp"

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