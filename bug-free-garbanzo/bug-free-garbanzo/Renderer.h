#pragma once

#include "Debug.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"




class Renderer
{
public:
	static bool Initialize();
	static bool InitializeGLFW();
	static bool InitializeGLAD();
	static bool InitializeIMGUI();

	static void PreRender();
	static void OnRender();
	static void PostRender();
	static void Dispose();

	static bool IsRunning();
	static bool IsInitialized();

	static void SetOpenGLVersion(int major, int minor);
	static void SetWindowName(const char* name);
	static void SetWindowSize(int width, int height);
private:
	static GLFWwindow* GLFW_WINDOW_CONTEXT;
	static ImGuiIO IO;

	static bool IS_INITIALIZED;

	static int OPENGL_VERSION_MAJOR, OPENGL_VERSION_MINOR;
	static std::string OPENGL_VERSION_STRING;

	static int WINDOW_SIZE_WIDTH, WINDOW_SIZE_HEIGHT;
};