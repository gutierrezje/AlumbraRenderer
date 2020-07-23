#include "pch.h"
#include "Texture.h"
#include <stb_image.h>

TextureLoader::TextureLoader() : m_textureID(0) {}

TextureLoader::~TextureLoader() {}

void TextureLoader::createNew(GLenum target, const TextureOptions& texOps)
{
    glCreateTextures(target, 1, &m_textureID);
    glTextureParameteri(m_textureID, GL_TEXTURE_MIN_FILTER, texOps.minFilter);
    glTextureParameteri(m_textureID, GL_TEXTURE_MAG_FILTER, texOps.magFilter);
    glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_S, texOps.wrapS);
    glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_T, texOps.wrapT);
    if (target == GL_TEXTURE_CUBE_MAP)
        glTextureParameteri(m_textureID, GL_TEXTURE_WRAP_R, texOps.wrapR);
}

GLuint TextureLoader::emptyTexture(GLenum format, GLsizei width, GLsizei height, GLsizei levels)
{
    glTextureStorage2D(m_textureID, levels, format, width, height);

    return m_textureID;
}

GLuint TextureLoader::fileTexture(const std::string& path)
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
            internalFormat = GL_SRGB8;
        }
        else if (nrComponents == 4) {
            format = GL_RGBA;
            internalFormat = GL_SRGB8_ALPHA8;
        }
        else
            std::cout << "Texture Error: Unsupported image format\n";

        glTextureStorage2D(m_textureID, 1, internalFormat, width, height);
        glTextureSubImage2D(m_textureID, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
        glGenerateTextureMipmap(m_textureID);

        stbi_image_free(data);
    }
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }
    return m_textureID;
}

void TextureLoader::bind(int index)
{
    glBindTextureUnit(index, m_textureID);
}