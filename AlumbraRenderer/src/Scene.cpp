#include "pch.h"
#include "Scene.h"

Scene::Scene()
    : m_cubemap(Cubemap())
{
    init();
}

Scene::~Scene() {
    for (auto& model : m_models)
        delete model;
    m_models.clear();
}

void Scene::init()
{
    m_models.push_back(new Model("res/models/nanosuit/nanosuit.obj"));
    m_models.push_back(new Model(Cube("res/textures/red.jpg")));
    m_models.push_back(new Model(Quad("res/textures/metal.png")));

    m_pLights = {
        PointLight{glm::vec4(2.0f, 1.0f, 1.0f, 0.0f), glm::vec4(1.0), 3.0f, 1.0, glm::vec2(0.0f)},
        PointLight{glm::vec4(-2.0f, 1.0, -1.0f, 0.0f), glm::vec4(1.0), 3.0f, 1.0, glm::vec2(0.0f)}
    };

    std::vector<std::string> faces
    {
        "res/cubemaps/mountainlake/right.jpg",
        "res/cubemaps/mountainlake/left.jpg",
        "res/cubemaps/mountainlake/top.jpg",
        "res/cubemaps/mountainlake/bottom.jpg",
        "res/cubemaps/mountainlake/front.jpg",
        "res/cubemaps/mountainlake/back.jpg"
    };
    m_cubemap.loadMap(faces);
}
