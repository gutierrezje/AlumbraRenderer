#include "pch.h"
#include "Cubemap.h"

Cubemap::Cubemap() {}

Cubemap::~Cubemap() {}

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

void Cubemap::captureEnvironment(const Framebuffer& captureBuffer, const Shader& captureShader)
{
    m_texOps.minFilter = GL_LINEAR_MIPMAP_LINEAR;
    m_texOps.magFilter = GL_LINEAR;
    m_texOps.wrapS = GL_CLAMP_TO_EDGE;
    m_texOps.wrapT = GL_CLAMP_TO_EDGE;
    m_texLoader.createNew(GL_TEXTURE_CUBE_MAP, m_texOps);
    m_environmentMap = m_texLoader.emptyTexture(GL_RGB16F, 2048, 2048);

    captureShader.use();
    captureShader.setSampler("equirectangularMap", 0);
    captureShader.setMat4("projection", captureProjection);
    glBindTextureUnit(0, m_cubemapID);

    glViewport(0, 0, 2048, 2048);
    captureBuffer.bindAs(GL_FRAMEBUFFER);
    for (unsigned face = 0; face < 6; face++) {
        captureShader.setMat4("view", captureViews[face]);
        glNamedFramebufferTextureLayer(captureBuffer.id(), GL_COLOR_ATTACHMENT0, m_environmentMap, 0, face);
        captureBuffer.clear();

        draw(captureShader);
    }
    glGenerateTextureMipmap(m_environmentMap);
}

void Cubemap::irradianceConvolution(const Framebuffer& captureBuffer, const Shader& convolveShader)
{
    m_texOps.minFilter = GL_LINEAR;
    m_texLoader.createNew(GL_TEXTURE_CUBE_MAP, m_texOps);
    m_irradianceMap = m_texLoader.emptyTexture(GL_RGB16F, 32, 32);
    captureBuffer.resizeRB(32, 32);

    convolveShader.use();
    convolveShader.setSampler("environmentMap", 0);
    convolveShader.setMat4("projection", captureProjection);
    glBindTextureUnit(0, m_environmentMap);

    glViewport(0, 0, 32, 32);
    for (unsigned face = 0; face < 6; face++) {
        convolveShader.setMat4("view", captureViews[face]);
        glNamedFramebufferTextureLayer(captureBuffer.id(), GL_COLOR_ATTACHMENT0, m_irradianceMap, 0, face);
        captureBuffer.clear();

        draw(convolveShader);
    }
}
void Cubemap::specularPrefilter(const Framebuffer& captureBuffer, const Shader& prefilterShader)
{
    unsigned maxMipLevels = 10;
    m_texOps.minFilter = GL_LINEAR_MIPMAP_LINEAR;
    m_texLoader.createNew(GL_TEXTURE_CUBE_MAP, m_texOps);
    m_prefilterMap = m_texLoader.emptyTexture(GL_RGB16F, 1024, 1024, maxMipLevels);
    glGenerateTextureMipmap(m_prefilterMap);

    prefilterShader.use();
    prefilterShader.setSampler("environmentMap", 0);
    prefilterShader.setMat4("projection", captureProjection);
    glBindTextureUnit(0, m_environmentMap);

    for (unsigned mip = 0; mip < maxMipLevels; mip++) {
        unsigned mipWidth = 1024 * std::pow(0.5, mip);
        unsigned mipHeight = 1024 * std::pow(0.5, mip);
        captureBuffer.resizeRB(mipWidth, mipHeight);
        glViewport(0, 0, mipWidth, mipHeight);

        float roughness = (float)mip / (float)(maxMipLevels - 1);
        prefilterShader.setFloat("roughness", roughness);
        for (unsigned face = 0; face < 6; face++) {
            prefilterShader.setMat4("view", captureViews[face]);
            glNamedFramebufferTextureLayer(captureBuffer.id(), GL_COLOR_ATTACHMENT0, m_prefilterMap, mip, face);
            captureBuffer.clear();

            draw(prefilterShader);
        }
    }
}
void Cubemap::brdfIntegrate(const Framebuffer& captureBuffer, const Shader& brdfIntegrateShader)
{
    
}

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
            std::cout << "Cubemap tex failed to load at path: " << faces[faceIdx] << std::endl;
            stbi_image_free(data);
        }
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
