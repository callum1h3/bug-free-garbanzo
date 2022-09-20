#include "Renderer.h"
#include "Input.h"
#include <string>

GLFWwindow* Renderer::GLFW_WINDOW_CONTEXT;
ImGuiIO Renderer::IO;
Camera Renderer::RENDERER_CAMERA;

unsigned int Renderer::gBuffer, Renderer::gPosition, Renderer::gNormal, Renderer::gColorSpec, Renderer::gAlbedoSpec, Renderer::rboDepth;
int Renderer::WINDOW_SIZE_WIDTH, Renderer::WINDOW_SIZE_HEIGHT;
int Renderer::OPENGL_VERSION_MAJOR, Renderer::OPENGL_VERSION_MINOR;
std::string Renderer::OPENGL_VERSION_STRING;

bool Renderer::IS_INITIALIZED = false;


std::map<unsigned int, Texture> Renderer::RENDERER_TEXTURES;
unsigned int Renderer::RENDERER_TEXTURE_COUNT;

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

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);

    ShaderManager::Initialize();
    InitializeGBuffer();

    // Camera Init
    Camera* _camera = GetCamera();
    _camera->Initialize();
    _camera->OnResolutionChange(WINDOW_SIZE_WIDTH, WINDOW_SIZE_HEIGHT);
    UpdateGBuffer();

    Input::Initialize(GLFW_WINDOW_CONTEXT);

    IS_INITIALIZED = true;
    Debug::Log("Successfully Initialized Renderer!");

    return true;
}

void Renderer::InitializeGBuffer()
{
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    // - position color buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 512, 512, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    // - normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 512, 512, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    // - color + specular color buffer
    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

    // - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 512, 512);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        Debug::Log("Frame Buffer not complete!");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    

    // Binding quad shader to textures
    Shader* shader = ShaderManager::GetShader("deferredshading");
    shader->Use();

    glUniform1i(glGetUniformLocation(shader->shader_main_program, "gPosition"), 0);
    glUniform1i(glGetUniformLocation(shader->shader_main_program, "gNormal"), 1);
    glUniform1i(glGetUniformLocation(shader->shader_main_program, "gAlbedoSpec"), 2);

    Debug::Log("Successfully Initialized GBuffer!");
}

void Renderer::UpdateGBuffer()
{
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WINDOW_SIZE_WIDTH, WINDOW_SIZE_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);

    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, WINDOW_SIZE_WIDTH, WINDOW_SIZE_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);

    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WINDOW_SIZE_WIDTH, WINDOW_SIZE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WINDOW_SIZE_WIDTH, WINDOW_SIZE_HEIGHT);
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
    glfwSetFramebufferSizeCallback(GLFW_WINDOW_CONTEXT, frameBufferCallback);

    Debug::Log("Successfully Initialized GLFW!");

    return true;
}

void Renderer::frameBufferCallback(GLFWwindow* _window, int width, int height)
{
    glViewport(0, 0, width, height);
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

void Renderer::PreRenderDef()
{
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::Update()
{
    Input::UpdateInput(WINDOW_SIZE_WIDTH, WINDOW_SIZE_HEIGHT);
    RENDERER_CAMERA.Update();
}

void Renderer::PostRenderDef()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    ShaderManager::GetShader("deferredshading")->Use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);

    RenderQuad();
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

        Camera* _camera = GetCamera();
        _camera->OnResolutionChange(width, height);

        UpdateGBuffer();
    }
}

Texture* Renderer::CreateTexture()
{
    return &RENDERER_TEXTURES[RENDERER_TEXTURE_COUNT];
    RENDERER_TEXTURE_COUNT++;
}

unsigned int Renderer::quadVAO = 0, Renderer::quadVBO = 0;
void Renderer::RenderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

bool Renderer::IsInitialized() { return IS_INITIALIZED; }
bool Renderer::IsRunning() { return !glfwWindowShouldClose(GLFW_WINDOW_CONTEXT); }
void Renderer::SetWindowName(const char* name) { glfwSetWindowTitle(GLFW_WINDOW_CONTEXT, name); }

Camera* Renderer::GetCamera() { return &RENDERER_CAMERA; }
GLFWwindow* Renderer::GetWindow() { return GLFW_WINDOW_CONTEXT; }