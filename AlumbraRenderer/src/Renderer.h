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
    Shader m_environmentShader;
    Shader m_fbShader;
    Shader m_directDepthShader;
    Shader m_pointDepthShader;
    Framebuffer m_fbo;
    GLuint m_fbQuadVAO;
    GLuint m_directionalDepthFBO;
    GLuint m_directionalDepthMap;
    std::vector<GLuint> m_pointDepthFBOs;
    std::vector<GLuint> m_pointDepthMaps;

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
