#include "Input.h"


	int Input::WINDOW_WIDTH;
	int Input::WINDOW_HEIGHT;

	double Input::MOUSE_X;
	double Input::MOUSE_Y;

	int Input::KEYBOARD_HORIZONTAL;
	int Input::KEYBOARD_VERTICAL;

	GLFWwindow* Input::GLFW_WINDOW;

	double Input::GetMouseX() { return MOUSE_X; }
	double Input::GetMouseY() { return MOUSE_Y; }
	double Input::GetKey(int key) { return glfwGetKey(GLFW_WINDOW, key); }

	int Input::GetHorizontal() { return KEYBOARD_HORIZONTAL; }
	int Input::GetVertical() { return KEYBOARD_VERTICAL; }

	void Input::GetMouseFPSStyle(double* x, double* y)
	{
		int new_x = Input::WINDOW_WIDTH / 2;
		int new_y = Input::WINDOW_HEIGHT / 2;

		glfwSetCursorPos(GLFW_WINDOW, new_x, new_y);

		*x = Input::MOUSE_X - new_x;
		*y = Input::MOUSE_Y - new_y;
	}

	void Input::Initialize(GLFWwindow* window)
	{
		GLFW_WINDOW = window;
	}

	void Input::UpdateInput(int window_width, int window_height)
	{
		WINDOW_WIDTH = window_width;
		WINDOW_HEIGHT = window_height;

		glfwGetCursorPos(GLFW_WINDOW, &MOUSE_X, &MOUSE_Y);

		KEYBOARD_HORIZONTAL = (GetKey(GLFW_KEY_D) == GLFW_PRESS) - (GetKey(GLFW_KEY_A) == GLFW_PRESS);
		KEYBOARD_VERTICAL = (GetKey(GLFW_KEY_W) == GLFW_PRESS) - (GetKey(GLFW_KEY_S) == GLFW_PRESS);
	}

	void Input::SetCursorMode(GLuint mode)
	{
		glfwSetInputMode(GLFW_WINDOW, GLFW_CURSOR, mode);
	}
