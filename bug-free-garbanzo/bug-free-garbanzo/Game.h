#pragma once
#include "Engine.h"
#include "VoxelManager.h"
#include "Debug.h"

class Game
{
public:
	static void Run();
	static void Start();
	static void Dispose();

	static void Render();
	static void PreRender();
	static void PostRender();

	static bool cursor_enable;
};