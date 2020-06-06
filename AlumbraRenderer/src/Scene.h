#pragma once

#include "mesh/Model.h"
#include "mesh/Shapes.h"
#include "FreeCamera.h"
#include "Cubemap.h"

struct PointLight {
    glm::vec4 position;
    glm::vec4 color;
    float radius;
    float intensity;
    glm::vec2 padding;
};

/**
 * Manages aspects of the scene that the renderer will draw
 */
class Scene
{
public:
    Scene();
    ~Scene();

    void init();
    inline std::vector<PointLight>& pointLights() { return m_pLights; }
    inline std::vector<Model*>& models() { return m_models; }
    inline Cubemap& cubemap() { return m_cubemap; }
private:
    FreeCamera m_camera;
    Cubemap m_cubemap;
    std::vector<PointLight> m_pLights;
    std::vector<Model*> m_models;
};
