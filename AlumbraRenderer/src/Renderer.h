#pragma once

#include "Scene.h"
#include "Texture.h"
#include "FreeCamera.h"
#include "Framebuffer.h"

/**
 * Class that is responsible for rendering our scene
 */
class Renderer {
public:
    Renderer(Scene* sceneView);
    ~Renderer();

    void beginDraw();
    void drawGUI();

private:
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

    Scene* m_scene;
    Shader m_modelShader;
    Shader m_gBufferShader;
    Shader m_cubemapCaptureShader;
    Shader m_cubemapConvolveShader;
    Shader m_cubemapPrefilterShader;
    Shader m_brdfPrecomputeShader;
    Shader m_skyboxShader;
    Shader m_postProcessShader;
    Shader m_directDepthShader;
    Shader m_pointDepthShader;
    Shader m_blurShader;

    Framebuffer m_mainBuffer, m_gBuffer, m_directDepthBuffer, m_captureBuffer, m_pingBuffer, m_pongBuffer;
    GLuint m_screenQuadVAO;
    
    // For shadows
    GLuint m_directionalDepthFBO;
    std::vector<GLuint> m_pointDepthFBOs;
    GLuint m_directionalDepthMap;
    std::vector<GLuint> m_pointDepthMaps;

    // TODO: make a cubemap class for these
    GLuint m_environmentMap;
    GLuint m_irradianceMap;
    GLuint m_prefilterMap;
    GLuint m_brdfLUT;

    // Settings
    // TODO: Add to Camera
    float m_exposure = 1.0f;
    // TODO: Material class
    glm::vec3 m_albedo = glm::vec3(1.0f, 0.782f, 0.344f);
    float m_roughness = 0.0f;
    float m_metallic = 1.0f;

    void setupShaders();
    void setupFramebuffers();
    void setupUniforms();
};

void messageCallback(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam);

const std::vector<glm::vec3> fbQuadPos{  
    glm::vec3(-1.0f,  1.0f, 0.0f),
    glm::vec3(-1.0f, -1.0f, 0.0f),
    glm::vec3( 1.0f, -1.0f, 0.0f),  

    glm::vec3(-1.0f,  1.0f, 0.0f),
    glm::vec3( 1.0f, -1.0f, 0.0f),
    glm::vec3( 1.0f,  1.0f, 0.0f)
};

const std::vector<glm::vec3> fbQuadNorm{
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(1.0f, 0.0f, 0.0f),

    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(1.0f, 1.0f, 0.0f)
};

const std::vector<glm::vec2> fbQuadTex{
    glm::vec2(0.0f, 1.0f),
    glm::vec2(0.0f, 0.0f),
    glm::vec2(1.0f, 0.0f),

    glm::vec2(0.0f, 1.0f),
    glm::vec2(1.0f, 0.0f),
    glm::vec2(1.0f, 1.0f),
};
