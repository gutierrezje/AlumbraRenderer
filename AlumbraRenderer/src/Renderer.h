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

    Framebuffer m_fbo;
    GLuint m_screenQuadVAO;
    
    // Framebuffers
    GLuint m_directionalDepthFBO;
    std::vector<GLuint> m_pointDepthFBOs;
    GLuint m_pingPongFBOs[2]{ 0,0 };
    GLuint m_gBufferFBO;
    GLuint m_captureFBO;

    // TODO: make a cubemap class for these
    GLuint m_environmentMap;
    GLuint m_irradianceMap;
    GLuint m_prefilterMap;
    GLuint m_brdfLUT;

    // Buffer attachments
    GLuint m_directionalDepthMap;
    std::vector<GLuint> m_pointDepthMaps;
    GLuint m_pingPongBuffers[2]{ 0,0 };
    GLuint m_gPosition;
    GLuint m_gNormal;
    GLuint m_gAlbedoSpec;
    GLuint m_gMetalRoughAO;
    GLuint m_gDepthMap;

    // Settings
    // TODO: Maybe this should be part of the Camera or Scene class
    float m_exposure = 1.0f;

    void setupShaders();
    void setupFramebuffers();
    void setupUBOs();
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
