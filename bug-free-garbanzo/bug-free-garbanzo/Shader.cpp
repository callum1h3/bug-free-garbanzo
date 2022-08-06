#include "Shader.h"
#include "Debug.h"
#include "SystemHelper.h"
#include "Renderer.h"

#include <filesystem>
#include <string>

void Shader::RegisterProgram(std::string fullpath, std::string extension)
{
    ShaderProgram program;
    program.program_name = fullpath;
    program.program_type_string = extension;

    programs.push_back(program);
}

// This compiles and deletes the old shader still need to write a shader modification system tho.
void Shader::Compile(std::string shaderFolderPath)
{
    // Compiles all shader files and checks for errors.
    bool hasErrors = false;
    for (int i = 0; i < programs.size(); i++)
    {
        ShaderProgram* program = &programs[i];
        program->program_id = glCreateShader(__extensionConvertion[program->program_type_string]);

        std::ifstream fileStream(shaderFolderPath + program->program_name + program->program_type_string + ".glsl");
        if (fileStream.good())
        {
            std::string programCode = std::string(std::istreambuf_iterator<char>(fileStream), std::istreambuf_iterator<char>());

            const char* c_str = programCode.c_str();
            glShaderSource(program->program_id, 1, &c_str, NULL);
            glCompileShader(program->program_id);

            int  success;
            char infoLog[512];
            glGetShaderiv(program->program_id, GL_COMPILE_STATUS, &success);

            if (!success)
            {
                hasErrors = true;
                glGetShaderInfoLog(program->program_id, 512, NULL, infoLog);
                Debug::Log("ERROR::SHADER::COMPILATION_FAILED::" + program->program_name + program->program_type_string + ": \n" + std::string(infoLog));

                continue;
            }
        }
    }

    // If the shaders have no errors it will compile the program.
    if (!hasErrors)
    {
        if (hasInitialized)
            glDeleteProgram(shader_main_program);
        shader_main_program = glCreateProgram();

        for (int i = 0; i < programs.size(); i++)
        {
            ShaderProgram* program = &programs[i];
            glAttachShader(shader_main_program, program->program_id);
        }

        glLinkProgram(shader_main_program);

        for (int i = 0; i < programs.size(); i++)
        {
            ShaderProgram* program = &programs[i];
            glDeleteShader(program->program_id);
        }

        int success;
        char infoLog[512];
        glGetProgramiv(shader_main_program, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader_main_program, 512, NULL, infoLog);
            Debug::Log("ERROR::SHADER::PROGRAM::LINKING_FAILED\n" + std::string(infoLog));
        }
        else
            Debug::Log("Successfully Compiled Shader Program: " + shader_name + "!");
    }
}

void Shader::Use()
{
    glUseProgram(shader_main_program);
}

void Shader::Dispose()
{
    glDeleteProgram(shader_main_program);
}

std::map<std::string, Shader> ShaderManager::SHADERS;
std::string ShaderManager::SHADER_PATH;

void ShaderManager::Initialize()
{
    SHADER_PATH = std::string(SystemHelper::PATH_DIRECTORY);
    SHADER_PATH.append("/shaders/");

    for (const auto& entry : std::filesystem::directory_iterator(SHADER_PATH.c_str()))
    {
        if (entry.path().extension().string() != ".glsl")
            continue;

        std::string filename = entry.path().stem().string();
        int fileNameLength = filename.length();
        int fileNameMinus = fileNameLength - 2;

        std::string shader_type = filename.substr(fileNameMinus, fileNameLength);
        filename.erase(fileNameMinus, fileNameLength);

        Shader* shader = GetShader(filename);
        shader->shader_name = filename;
        shader->RegisterProgram(filename, shader_type);
    }

    CompileShaders();
    Debug::Log("Successfully Initialized Shaders!");
}

void ShaderManager::CompileShaders()
{
    for (const auto& [key, value] : SHADERS)
    {
        Shader* shader = &SHADERS[key];
        shader->Compile(SHADER_PATH);
        shader->hasInitialized = true;

        Renderer::GetCamera()->BindViewBuffer(shader->shader_main_program);
    }
}

Shader* ShaderManager::GetShader(std::string shader_name) { return &SHADERS[shader_name]; }