#pragma once

#include "GLFW/glfw3.h"


	class Input
	{
	private:
		static int WINDOW_WIDTH;
		static int WINDOW_HEIGHT;

		static double MOUSE_X;
		static double MOUSE_Y;

		static int KEYBOARD_HORIZONTAL;
		static int KEYBOARD_VERTICAL;

		static GLFWwindow* GLFW_WINDOW;

	public:
		static double GetMouseX();
		static double GetMouseY();
		static double GetKey(int key);

		static int GetHorizontal();
		static int GetVertical();

		static void GetMouseFPSStyle(double* x, double* y);
		static void Initialize(GLFWwindow* window);
		static void UpdateInput(int window_width, int window_height);

		static void SetCursorMode(GLuint mode);
		static void SetTargetCursorMode(GLuint mode);
	};
