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
    void beginDraw();
    void drawGUI();

private:
    Scene* m_scene;
    Shader m_modelShader;
    Shader m_cubemapShader;
    Shader m_fbShader;
    Framebuffer m_fbo;
    unsigned int m_fbQuadVAO;
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
