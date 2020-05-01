#include "pch.h"
#include "Cubemap.h"

Cubemap::Cubemap()
    : m_vbo(Buffer(sizeof(cubemapVertices), 36, 1))
    , m_vao(VertexArray())
{
    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &m_cubemapID);

    glTextureParameteri(m_cubemapID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(m_cubemapID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_cubemapID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_cubemapID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(m_cubemapID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    m_vbo.addData(cubemapVertices, sizeof(cubemapVertices));
    m_vao.useBuffer(m_vbo);
}

Cubemap::~Cubemap() {}

void Cubemap::loadMap(const std::vector<std::string>& faces)
{
    int width, height, nrChannels;
    // Need to load first face in order to get width/height for the storage function
    unsigned char* data = stbi_load(faces[0].c_str(), &width, &height, &nrChannels, 0);
    if (data) {
        glTextureStorage2D(m_cubemapID, 1, GL_RGB8, width, height);
        glTextureSubImage3D(m_cubemapID, 0, 0, 0, 0, width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, data);
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
            glTextureSubImage3D(m_cubemapID, 0, 0, 0, faceIdx, width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else {
            // TODO: Log
            std::cout << "Cubemap tex failed to load at path: " << faces[faceIdx] << std::endl;
            stbi_image_free(data);
        }
    }
}

void Cubemap::draw(const Shader& shader)
{
    glDepthFunc(GL_LEQUAL);
    bind();
    m_vao.bind();
    shader.setSampler("skybox", 0);
    glDrawArrays(GL_TRIANGLES, 0, m_vbo.vertexCount());
    glDepthFunc(GL_LESS);
}

void Cubemap::bind()
{
    glBindTextureUnit(0, m_cubemapID);
}
