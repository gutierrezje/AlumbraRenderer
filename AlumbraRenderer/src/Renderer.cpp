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
    
  currentShader.setFloat("material.shininess", 128.0f);
    
  currentShader.setVec3("pointLight.position", -1.5f, 1.5f, 1.5f);
  currentShader.setVec3("pointLight.color", 1.0f, 1.0f, 1.0f);
  currentShader.setFloat("pointLight.radius", 5.0f);
  currentShader.setFloat("pointLight.intensity", 5.0f);
    
  currentShader.setMat4("projection", projectionM);
  currentShader.setMat4("view", viewM);
  

  const auto& models = m_scene->models();
	for (auto model : models) {
    // render the loaded model
    glm::mat4 modelM = glm::mat4(1.0f);
    //modelM = glm::translate(modelM, glm::vec3(0.0f, 0.0f, 0.0f));
    //modelM = glm::scale(modelM, glm::vec3(0.2f, 0.2f, 0.2f));
    
    currentShader.setMat4("model", modelM);
		model->draw(currentShader);
	}

  glDepthFunc(GL_LEQUAL);
  glm::mat4 cmView = glm::mat4(glm::mat3(viewM));
  m_cubemapShader.use();
  m_cubemapShader.setMat4("projection", projectionM);
  m_cubemapShader.setMat4("view", cmView);
  m_scene->cubemap().draw(m_cubemapShader);
  glDepthFunc(GL_LESS);
}

