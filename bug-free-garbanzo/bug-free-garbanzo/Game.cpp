#include "Game.h"

bool Game::cursor_enable = true;

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

	VoxelManager::Initialize();
}

void Game::Render()
{
	VoxelManager::Update();
	Renderer::OnRender();

	if (Input::GetKey(GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		cursor_enable = true;
		Input::SetCursorMode(GLFW_CURSOR_NORMAL);
	}

	if (Input::GetKey(GLFW_KEY_E) == GLFW_PRESS)
	{
		cursor_enable = false;
		Input::SetCursorMode(GLFW_CURSOR_DISABLED);
	}

	Camera* camera = Renderer::GetCamera();
	if (!cursor_enable)
	{
		double mouse_x, mouse_y;
		Input::GetMouseFPSStyle(&mouse_x, &mouse_y);
		camera->AddAngles(mouse_x * 0.01, -mouse_y * 0.01);
	}


	glm::vec3 direction = glm::vec3(0, 0, 0);
	direction += camera->Forward() *= Input::GetVertical();
	direction += camera->Right() *= Input::GetHorizontal();

	camera->SetPosition(camera->GetPosition() + (direction *= 0.2f));
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
	VoxelManager::Dispose();
	Renderer::Dispose();
}