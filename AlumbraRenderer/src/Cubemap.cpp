#include "pch.h"
#include "Cubemap.h"

Cubemap::Cubemap() {}

Cubemap::~Cubemap() {}

void Cubemap::loadMap(const std::vector<std::string>& faces)
{
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_cubemapID);

    glTextureParameteri(m_cubemapID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_cubemapID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_cubemapID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_cubemapID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_cubemapID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    DataBuffer buffer(sizeof(cubemapVertices[0]) * cubemapVertices.size(), 36, 1);
    buffer.addVec3s(cubemapVertices.data());

    VertexArray vao;
    vao.loadBuffer(buffer, -1);
    m_vao = vao.vertexArrayID();
    int width, height, nrChannels;
    // Need to load first face in order to get width/height for the storage function
    unsigned char* data = stbi_load(faces[0].c_str(), &width, &height, &nrChannels, 0);
    GLenum internalFormat;
    GLenum format;
    if (nrChannels == 3) {
        internalFormat = GL_SRGB8;
        format = GL_RGB;
    }
    else if (nrChannels == 4) {
        internalFormat = GL_SRGB8_ALPHA8;
        format = GL_RGBA;
    }
    if (data) {
        glTextureStorage2D(m_cubemapID, 1, internalFormat, width, height);
        glTextureSubImage3D(m_cubemapID, 0, 0, 0, 0, width, height, 1, format, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }
    else {
        // TODO: Log
        std::cout << "Cubemap tex failed to load at path: " << 0 << std::endl;
        stbi_image_free(data);
        return;
    }
    for (unsigned int faceIdx = 1; faceIdx < faces.size(); ++faceIdx)
    {
        data = stbi_load(faces[faceIdx].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTextureSubImage3D(m_cubemapID, 0, 0, 0, faceIdx, width, height, 1, format, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else {
            // TODO: Log
            std::cout << "Cubemap tex failed to load at path: " << faces[faceIdx] << std::endl;
            stbi_image_free(data);
        }
    }
}

void Cubemap::loadHDRMap(const std::string& hdrImage)
{
    DataBuffer buffer(sizeof(cubemapVertices[0]) * cubemapVertices.size(), 36, 1);
    buffer.addVec3s(cubemapVertices.data());

    VertexArray vao;
    vao.loadBuffer(buffer, -1);
    m_vao = vao.vertexArrayID();

    stbi_set_flip_vertically_on_load(true);
    int width, height, nrComponents;
    float* data = stbi_loadf(hdrImage.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        glCreateTextures(GL_TEXTURE_2D, 1, &m_cubemapID);
        glTextureStorage2D(m_cubemapID, 1, GL_RGB16F, width, height);
        glTextureSubImage2D(m_cubemapID, 0, 0, 0, width, height, GL_RGB, GL_FLOAT, data);

        glTextureParameteri(m_cubemapID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_cubemapID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTextureParameteri(m_cubemapID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTextureParameteri(m_cubemapID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else {
        std::cout << "Failed to load HDR image.\n";
    }
}

void Cubemap::draw(const Shader& shader)
{
    glDepthFunc(GL_LEQUAL);
    //shader.setSampler("cubeMap", 0);
    //glBindTextureUnit(0, m_cubemapID);
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthFunc(GL_LESS);
}
