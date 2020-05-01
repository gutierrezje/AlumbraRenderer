#include "pch.h"
#include "Renderer.h"
#include "Shader.h"
#include "Window.h"
#include <stb_image.h>

Renderer::Renderer(Scene* sceneView)
    : m_scene(sceneView)
    , m_modelShader(Shader("src/shaders/basic.vert", "src/shaders/basic.frag"))
    , m_cubemapShader(Shader("src/shaders/cubemap.vert", "src/shaders/cubemap.frag"))
    , m_enviroShader(Shader("src/shaders/environment.vert", "src/shaders/environment.frag"))
{
    init();
}

Renderer::~Renderer() {}

void Renderer::init()
{
    // Enable certain features
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
}

void Renderer::drawScene(bool useEnviro)
{
    glm::mat4 projectionM = glm::perspective(glm::radians(g_camera.zoom()),
        (float)Window::width() / (float)Window::height(), 0.1f, 100.0f);
    glm::mat4 viewM = g_camera.getViewMatrix();

    Shader& currentShader = useEnviro ? m_enviroShader : m_modelShader;
    currentShader.use();
    if (useEnviro) {
        currentShader.setSampler("cubemap", 0);
    }

    currentShader.setVec3("viewPos", g_camera.position());

    currentShader.setFloat("material.shininess", 32.0f);

    // Directional light
    currentShader.setVec3("directLight.direction", 0.2f, -1.0f, 0.3f);
    currentShader.setVec3("directLight.color", 1.0f, 1.0f, 1.0f);
    currentShader.setFloat("directLight.intensity", 1.0f);

    // Point lights
    currentShader.setInt("numPointLights", 0);
    /*currentShader.setVec3("pointLights[0].position", -1.5f, 1.5f, 1.5f);
    currentShader.setVec3("pointLights[0].color", 1.0f, 1.0f, 1.0f);
    currentShader.setFloat("pointLights[0].radius", 5.0f);
    currentShader.setFloat("pointLights[0].intensity", 5.0f);*/

    // Spotlight
    currentShader.setVec3("spotLight.position", g_camera.position());
    currentShader.setVec3("spotLight.direction", g_camera.front());
    currentShader.setVec3("spotLight.color", 1.0f, 1.0f, 1.0f);
    currentShader.setFloat("spotLight.radius", 10.0f);
    currentShader.setFloat("spotLight.intensity", 2.0f);
    currentShader.setFloat("spotLight.penumbra", glm::cos(glm::radians(12.5f)));
    currentShader.setFloat("spotLight.umbra", glm::cos(glm::radians(15.0f)));

    currentShader.setMat4("projection", projectionM);
    currentShader.setMat4("view", viewM);

    // Draw models
    const auto& models = m_scene->models();
    for (auto model : models) {
        glm::mat4 modelM = glm::mat4(1.0f);
        //modelM = glm::scale(modelM, glm::vec3(0.2f, 0.2f, 0.2f));

        currentShader.setMat4("model", modelM);
        model->draw(currentShader);
    }

    // Draw cubemap

    glm::mat4 cmView = glm::mat4(glm::mat3(viewM));
    m_cubemapShader.use();
    m_cubemapShader.setMat4("projection", projectionM);
    m_cubemapShader.setMat4("view", cmView);
    m_scene->cubemap().draw(m_cubemapShader);
}

