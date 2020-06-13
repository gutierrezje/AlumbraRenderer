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

    void init();
    unsigned int setupUBOs();
    void beginDraw();
    void drawGUI();

private:
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

    const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
};

void messageCallback(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam);

const float fbQuadPos[] = {  
    -1.0f,  1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,  
  
    -1.0f,  1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
     1.0f,  1.0f, 0.0f
};

const float fbQuadNorm[]{
    0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
                
    1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 0.0f
};

const float fbQuadTex[]{
    0.0f, 1.0f,
    0.0f, 0.0f,
    1.0f, 0.0f,
               
    0.0f, 1.0f,
    1.0f, 0.0f,
    1.0f, 1.0f,
};
