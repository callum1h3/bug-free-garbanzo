#pragma once

#include "Renderer.h"

class Game
{
public:
	static void Run();
	static void Start();
	static void Dispose();

	static void Render();
	static void PreRender();
	static void PostRender();
};