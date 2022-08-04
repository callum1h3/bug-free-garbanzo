#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <vector>
#include <map>
#include <fstream>
#include <filesystem>

static std::map< std::string, GLenum > __extensionConvertion = {
    {"_v", GL_VERTEX_SHADER},
    {"_f", GL_FRAGMENT_SHADER}
};

struct ShaderProgram
{
public:
    std::string program_name;
    std::string program_type_string;
    unsigned    program_id;
    int         program_type;
};

class Shader
{
public:
    void RegisterProgram(std::string fullpath, std::string extension);
    void Compile(std::string shaderFolderPath);
    void Use();
    void Dispose();

    unsigned int shader_main_program;
    bool hasInitialized = false;
    std::string shader_name;
private:
    std::vector<ShaderProgram> programs;
};

class ShaderManager
{
public:
    static void Initialize();
    static void CompileShaders();
    static Shader* GetShader(std::string shader_name);
private:
    static std::map<std::string, Shader> SHADERS;
    static std::string SHADER_PATH;
};