#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"

struct Texture
{
public:
    unsigned int BUFFER;
    GLuint BUFFER_TYPE;

    void Initialize(GLuint buffer_type);
    void Use();
    void Dispose();

    void SetPointFilter();
    void SetTextureArray(unsigned char* data, int width, int height, GLenum image_format, GLenum value_type = GL_UNSIGNED_BYTE);
};
