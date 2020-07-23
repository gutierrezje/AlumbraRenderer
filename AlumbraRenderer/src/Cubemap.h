#pragma once
#include "Shader.h"
#include "Buffers.h"
#include "Framebuffer.h"
#include "Texture.h"
#include <stb_image.h>

class Cubemap {
public:
    Cubemap();
    ~Cubemap();

    void loadHDRMap(const std::string& hdrImage);
    void captureEnvironment(const Framebuffer& captureBuffer, const Shader& captureShader);
    void irradianceConvolution(const Framebuffer& captureBuffer, const Shader& convolveShader);
    void specularPrefilter(const Framebuffer& captureBuffer, const Shader& prefilterShader);
    void brdfIntegrate(const Framebuffer& captureBuffer, const Shader& brdfIntegrateShader);

    void loadMap(const std::vector<std::string>& faces);
    void draw(const Shader& shader);

    inline GLuint cubemapID() const { return m_cubemapID; }
    inline GLuint environmentMap() const { return m_environmentMap; }
    inline GLuint irradianceMap() const { return m_irradianceMap; }
    inline GLuint prefilterMap() const { return m_prefilterMap; }
    inline GLuint vao() const { return m_vao; }
private:
    GLuint m_cubemapID;
    GLuint m_environmentMap;
    GLuint m_irradianceMap;
    GLuint m_prefilterMap;
    GLuint m_vao;

    TextureLoader m_texLoader;
    TextureOptions m_texOps;

    const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    const glm::mat4 captureViews[6]{
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
        glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),
    };
};

const std::vector<glm::vec3> cubemapVertices{
    // positions        
    glm::vec3(-1.0f,  1.0f, -1.0f),
    glm::vec3(-1.0f, -1.0f, -1.0f),
    glm::vec3( 1.0f, -1.0f, -1.0f),
    glm::vec3( 1.0f, -1.0f, -1.0f),
    glm::vec3( 1.0f,  1.0f, -1.0f),
    glm::vec3(-1.0f,  1.0f, -1.0f),

    glm::vec3(-1.0f, -1.0f,  1.0f),
    glm::vec3(-1.0f, -1.0f, -1.0f),
    glm::vec3(-1.0f,  1.0f, -1.0f),
    glm::vec3(-1.0f,  1.0f, -1.0f),
    glm::vec3(-1.0f,  1.0f,  1.0f),
    glm::vec3(-1.0f, -1.0f,  1.0f),

    glm::vec3( 1.0f, -1.0f, -1.0f),
    glm::vec3( 1.0f, -1.0f,  1.0f),
    glm::vec3( 1.0f,  1.0f,  1.0f),
    glm::vec3( 1.0f,  1.0f,  1.0f),
    glm::vec3( 1.0f,  1.0f, -1.0f),
    glm::vec3( 1.0f, -1.0f, -1.0f),

    glm::vec3(-1.0f, -1.0f,  1.0f),
    glm::vec3(-1.0f,  1.0f,  1.0f),
    glm::vec3( 1.0f,  1.0f,  1.0f),
    glm::vec3( 1.0f,  1.0f,  1.0f),
    glm::vec3( 1.0f, -1.0f,  1.0f),
    glm::vec3(-1.0f, -1.0f,  1.0f),

    glm::vec3(-1.0f,  1.0f, -1.0f),
    glm::vec3( 1.0f,  1.0f, -1.0f),
    glm::vec3( 1.0f,  1.0f,  1.0f),
    glm::vec3( 1.0f,  1.0f,  1.0f),
    glm::vec3(-1.0f,  1.0f,  1.0f),
    glm::vec3(-1.0f,  1.0f, -1.0f),

    glm::vec3(-1.0f, -1.0f, -1.0f),
    glm::vec3(-1.0f, -1.0f,  1.0f),
    glm::vec3( 1.0f, -1.0f, -1.0f),
    glm::vec3( 1.0f, -1.0f, -1.0f),
    glm::vec3(-1.0f, -1.0f,  1.0f),
    glm::vec3( 1.0f, -1.0f,  1.0f)
};

