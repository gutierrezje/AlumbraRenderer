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
    //m_transforms.push_back(Transform{ glm::vec3(0.0f), glm::vec3(0.2f), glm::vec3(-2.0f, 0.15f, -2.0f) }); });

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

void Scene::update(float deltaTime, const std::vector<InputEvent>& inputEvents)
{
    static int lastX{ 0 }, lastY{ 0 };
    
    for (auto& event : inputEvents) {
        if (event.inputType == InputType::move) {
            int deltaX = event.mouseX - lastX;
            int deltaY = lastY - event.mouseY;
            lastX = event.mouseX;
            lastY = event.mouseY;
        }
    }
}

Scene::~Scene() {
    for (auto& model : m_models)
        delete model;
    m_models.clear();
}
static const char* inputTypeString(InputType type)
{
    switch (type) {
    case InputType::scroll: return "scroll";
    case InputType::move: return "move";
    case InputType::press: return "press";
    case InputType::release: return "release";
    }
}

static const char* inputPressedString(std::variant<Key, Mouse> inputPressed)
{
    if (Key* keyPressed = std::get_if<Key>(&inputPressed)) {
        switch (*keyPressed) {
        case Key::a: return "A";
        case Key::c: return "C";
        case Key::d: return "D";
        case Key::m: return "M";
        case Key::s: return "S";
        case Key::w: return "W";
        case Key::space: return "SPC";
        case Key::esc: return "ESC";
        default: return "Something else";
        }
    }
    else if (Mouse* mousePressed = std::get_if<Mouse>(&inputPressed)) {
        switch (*mousePressed) {
        case Mouse::left: return "left";
        case Mouse::right: return "right";
        case Mouse::middle: return "middle";
        case Mouse::none    : return "none";
        }
    }

}
