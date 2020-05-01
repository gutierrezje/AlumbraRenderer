#include "pch.h"
#include "Texture.h"
#include <stb_image.h>

Texture::Texture()
{
    glCreateTextures(GL_TEXTURE_2D, 1, &m_textureID);

    glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

Texture::~Texture() {}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
void Texture::loadTexture(const std::string& path, bool gamma)
{
    m_path = path;
    int width, height, nrComponents;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        GLenum internalFormat, format;
        if (nrComponents == 1) {
            format = GL_RED;
            internalFormat = GL_R8;
        }
        else if (nrComponents == 3) {
            format = GL_RGB;
            internalFormat = GL_RGB4;
        }
        else if (nrComponents == 4) {
            format = GL_RGBA;
            internalFormat = GL_RGBA4;
        }
        else
            // TODO: Log
            std::cout << "Texture Error: Unsupported image format\n";

        glTextureStorage2D(m_textureID, 1, internalFormat, width, height);
        glTextureSubImage2D(m_textureID, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
        glGenerateTextureMipmap(m_textureID);

        stbi_image_free(data);
    }
    else {
        // TODO: Log
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }
}

void Texture::bind(int index)
{
    glBindTextureUnit(index, m_textureID);
}