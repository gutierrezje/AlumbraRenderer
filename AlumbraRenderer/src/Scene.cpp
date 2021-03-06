#include "pch.h"
#include "Scene.h"

Scene::Scene()
    : m_cubemap(Cubemap())
    , m_dLight(DirectionalLight{ glm::vec3(2.0f, -4.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 7.5 })
{
    // Setting up models and their transforms
    //m_models.push_back(new Model("res/models/nanosuit/nanosuit.obj"));
    //m_transforms.push_back(Transform{ glm::vec3(0.0f), glm::vec3(0.2f), glm::vec3(0.0f, -0.5f, 0.0f) });
    //m_models.push_back(new Model(Cube("res/textures/red.jpg")));
    //m_transforms.push_back(Transform{ glm::vec3(0.0f), glm::vec3(0.2f), glm::vec3(-2.0f, 0.15f, -2.0f) });
    //m_models.push_back(new Model(Cube("res/textures/white.jpg")));
    //m_transforms.push_back(Transform{ glm::vec3(0.0f), glm::vec3(0.2f), glm::vec3(2.0f, 0.15f, -2.0f) });
    //m_models.push_back(new Model(Cube("res/textures/brickwall.jpg", "", "res/textures/brickwall_normal.jpg")));
    //m_transforms.push_back(Transform{ glm::vec3(0.0f), glm::vec3(0.75f), glm::vec3(2.0f, 0.5f, 2.0f) });
    //m_models.push_back(new Model(Cube("res/textures/green.jpg")));
    //m_transforms.push_back(Transform{ glm::vec3(0.0f), glm::vec3(0.2f), glm::vec3(-2.0f, 0.15f, 2.0f) });
    //m_models.push_back(new Model(Quad("res/textures/metal.png")));
    //m_transforms.push_back(Transform{ glm::vec3(0.0f), glm::vec3(2.0f, 1.0f, 2.0f), glm::vec3(0.0f, 0.4f, 0.0f) });

    m_models.push_back(new Model(Sphere()));

    // Setting up point lights
    m_pLights = {
        PointLight{glm::vec4(-10.0f, 10.0f, 10.0f, 0.0f), 
            glm::vec4(1.0, 1.0, 1.0, 1.0),
            10.0f, 300.0, glm::vec2(0.0f)},
        PointLight{glm::vec4(10.0f, 10.0f, 10.0f, 0.0f), 
            glm::vec4(1.0, 1.0, 1.0, 1.0),
            10.0f, 300.0, glm::vec2(0.0f)},
        PointLight{glm::vec4(-10.0f, -10.0, 10.0f, 0.0f),
            glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
            10.0f, 300.0, glm::vec2(0.0f)},
        PointLight{glm::vec4(10.0f, -10.0f, 10.0f, 0.0f),
            glm::vec4(1.0, 1.0, 1.0, 1.0),
            10.0f, 300.0, glm::vec2(0.0f)}
    };

    // Setting up cubemap
    /*std::vector<std::string> faces
    {
        "res/cubemaps/quattro/px.png",
        "res/cubemaps/quattro/nx.png",
        "res/cubemaps/quattro/py.png",
        "res/cubemaps/quattro/ny.png",
        "res/cubemaps/quattro/pz.png",
        "res/cubemaps/quattro/nz.png"
    };
    m_cubemap.loadMap(faces);*/
    m_cubemap.loadHDRMap("res/cubemaps/quattro_canti_8k.hdr");
}

Scene::~Scene() {
    for (auto& model : m_models)
        delete model;
    m_models.clear();
}
