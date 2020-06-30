#pragma once

#include "mesh/Model.h"
#include "mesh/Shapes.h"
#include "FreeCamera.h"
#include "Cubemap.h"

struct DirectionalLight {
    glm::vec3 direction;
    glm::vec3 color;
    float intensity;
    float farPlane;
};

struct PointLight {
    glm::vec4 position;
    glm::vec4 color;
    float intensity;
    float radius;
    glm::vec2 padding;
};

struct Transform {
    glm::vec3 rotate    = glm::vec3(0.0f);
    glm::vec3 scale     = glm::vec3(0.0f);
    glm::vec3 translate = glm::vec3(0.0f);
};

/**
 * Manages aspects of the scene that the renderer will draw
 */
class Scene
{
public:
    Scene();
    ~Scene();

    inline DirectionalLight& directionalLight() { return m_dLight; }
    inline std::vector<PointLight>& pointLights() { return m_pLights; }
    inline std::vector<Model*>& models() { return m_models; }
    inline std::vector<Transform>& transforms() { return m_transforms; }
    inline Cubemap& cubemap() { return m_cubemap; }
private:
    FreeCamera m_camera;
    Cubemap m_cubemap;
    DirectionalLight m_dLight;
    std::vector<PointLight> m_pLights;
    std::vector<Model*> m_models;
    std::vector<Transform> m_transforms;
};
