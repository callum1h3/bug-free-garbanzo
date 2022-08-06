#include "Buffers.h"

void Texture::Initialize(GLuint buffer_type)
{
    BUFFER_TYPE = buffer_type;
    glGenTextures(1, &BUFFER);
}

void Texture::Use()
{
    glBindTexture(BUFFER_TYPE, BUFFER);
}

void Texture::SetPointFilter()
{
    glBindTexture(BUFFER_TYPE, BUFFER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(BUFFER_TYPE, 0);
}

void Texture::SetTextureArray(unsigned char* data, int width, int height, GLenum image_format, GLenum value_type)
{
    glBindTexture(BUFFER_TYPE, BUFFER);
    glTexImage3D(BUFFER_TYPE, 0, image_format, width, height, 1, 0, image_format, value_type, data);
    glBindTexture(BUFFER_TYPE, 0);
}