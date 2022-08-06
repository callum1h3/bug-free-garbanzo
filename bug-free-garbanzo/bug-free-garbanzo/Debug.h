#pragma once

#include <iostream>
#include <vector>
#include <string>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

class Debug
{
public:
	static void Initialize(GLFWwindow* window);
	static void Render();
	static void PreRender();
	static void Dispose();

	static ImGuiIO IO;
	static bool VSYNC_TOGGLE;
	static bool WIREFRAME_TOGGLE;
	static double LAST_FRAME_TIME;

	static float FPS_LARGEST_FRAME;
	static std::vector<float> FPS_STORAGE;

	static void Log(const char* input_text);
	static void Log(std::string input_text);
};