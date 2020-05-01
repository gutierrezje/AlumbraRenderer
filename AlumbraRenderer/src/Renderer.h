#pragma once

#include "Scene.h"
#include "Texture.h"
#include "FreeCamera.h"

/**
 * Class that is responsible for rendering our scene
 */
class Renderer {
public:
    Renderer(Scene* sceneView);
    ~Renderer();

    void init();
    void drawScene(bool useEnviro = false);

private:
    Scene* m_scene;
    Shader m_modelShader;
    Shader m_cubemapShader;
    Shader m_enviroShader;
};