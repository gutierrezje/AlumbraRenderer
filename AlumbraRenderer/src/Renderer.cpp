#include "pch.h"
#include "Renderer.h"
#include "Shader.h"
#include "Window.h"
#include <stb_image.h>

Renderer::Renderer(Scene* sceneView) 
  : m_sceneView(sceneView)
  , shader(Shader("src/shaders/basic.vert", "src/shaders/basic.frag"))
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

void Renderer::drawScene()
{
	shader.use();
  shader.setVec3("viewPos", g_camera.position());
  shader.setFloat("material.shininess", 128.0f);

  shader.setVec3("pointLight.position", -1.5f, 1.5f, 1.5f);
  shader.setVec3("pointLight.color", 1.0f, 1.0f, 1.0f);
  shader.setFloat("pointLight.radius", 5.0f);
  shader.setFloat("pointLight.intensity", 5.0f);
  const auto& models = m_sceneView->models();
	for (auto model : models) {

    glm::mat4 projectionM = glm::perspective(glm::radians(g_camera.zoom()), 
      (float)Window::width() / (float)Window::height(), 0.1f, 100.0f);
    glm::mat4 viewM = g_camera.getViewMatrix();
    shader.setMat4("projection", projectionM);
    shader.setMat4("view", viewM);

    // render the loaded model
    glm::mat4 modelM = glm::mat4(1.0f);
    //modelM = glm::translate(modelM, glm::vec3(0.0f, 0.0f, 0.0f));
    modelM = glm::scale(modelM, glm::vec3(0.2f, 0.2f, 0.2f));	// it's a bit too big for our scene, so scale it down
    
    shader.setMat4("model", modelM);
		model->draw(shader);
	}
}

