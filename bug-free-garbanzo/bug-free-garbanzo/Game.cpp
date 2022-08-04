#include "Game.h"
#include "SystemHelper.h"

void Game::Run()
{
	SystemHelper::Initialize();
	Renderer::SetOpenGLVersion(4, 3);

	if (!Renderer::Initialize())
		return;

	Start();

	while (Renderer::IsRunning())
	{
		PreRender();
		Render();
		PostRender();
	}

	Dispose();
}

void Game::Start()
{
	Renderer::SetWindowSize(1920, 1080);
	Renderer::SetWindowName("EPIC ENGINE");
}

void Game::Render()
{
	Renderer::OnRender();
}

void Game::PreRender()
{
	Renderer::PreRender();
}

void Game::PostRender()
{
	Renderer::PostRender();
}

void Game::Dispose()
{
	Renderer::Dispose();
}