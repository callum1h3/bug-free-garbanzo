#include "Renderer.h"
#include "Shader.h"

#include <string>

GLFWwindow* Renderer::GLFW_WINDOW_CONTEXT;
ImGuiIO Renderer::IO;

int Renderer::WINDOW_SIZE_WIDTH, Renderer::WINDOW_SIZE_HEIGHT;
int Renderer::OPENGL_VERSION_MAJOR, Renderer::OPENGL_VERSION_MINOR;
std::string Renderer::OPENGL_VERSION_STRING;

bool Renderer::IS_INITIALIZED = false;

bool Renderer::Initialize()
{
    OPENGL_VERSION_STRING = "#version ";
    OPENGL_VERSION_STRING.append(std::to_string(OPENGL_VERSION_MAJOR));
    OPENGL_VERSION_STRING.append(std::to_string(OPENGL_VERSION_MINOR));
    OPENGL_VERSION_STRING.append(std::to_string(0));

    if (!InitializeGLFW())
        return false;
    
    if (!InitializeGLAD())
        return false;

    if (!InitializeIMGUI())
        return false;

    ShaderManager::Initialize();

    IS_INITIALIZED = true;

    Debug::Log("Successfully Initialized Renderer!");

    return true;
}

bool Renderer::InitializeGLFW()
{
    if (!glfwInit())
        return false;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_VERSION_MAJOR);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_VERSION_MINOR);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    GLFW_WINDOW_CONTEXT = glfwCreateWindow(640, 480, "UNTITLED", NULL, NULL);
    if (!GLFW_WINDOW_CONTEXT)
    {
        Debug::Log("ERROR: Failed to initialize GLFW context!");
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(GLFW_WINDOW_CONTEXT);

    Debug::Log("Successfully Initialized GLFW!");

    return true;
}

bool Renderer::InitializeGLAD()
{
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        Debug::Log("ERROR: Failed to load GLAD");
        return false;
    }

    Debug::Log("Successfully Initialized GLAD!");

    return true;
}

bool Renderer::InitializeIMGUI()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    IO = ImGui::GetIO();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(GLFW_WINDOW_CONTEXT, true);
    ImGui_ImplOpenGL3_Init(OPENGL_VERSION_STRING.c_str());

    Debug::Log("Successfully Initialized IMGUI!");

    return true;
}

void Renderer::PreRender()
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::OnRender()
{

}

void Renderer::PostRender()
{
    glfwSwapBuffers(GLFW_WINDOW_CONTEXT);
    glfwPollEvents();
}

void Renderer::Dispose()
{
    Debug::Log("Disposing Renderer!");
    glfwTerminate();
}

void Renderer::SetOpenGLVersion(int major, int minor)
{
    OPENGL_VERSION_MAJOR = major;
    OPENGL_VERSION_MINOR = minor;
}

void Renderer::SetWindowSize(int width, int height)
{
    WINDOW_SIZE_WIDTH = width;
    WINDOW_SIZE_HEIGHT = height;

    if (IsInitialized())
    {
        glfwSetWindowSize(GLFW_WINDOW_CONTEXT, width, height);
    }
}

bool Renderer::IsInitialized() { return IS_INITIALIZED; }
bool Renderer::IsRunning() { return !glfwWindowShouldClose(GLFW_WINDOW_CONTEXT); }
void Renderer::SetWindowName(const char* name) { glfwSetWindowTitle(GLFW_WINDOW_CONTEXT, name); }
