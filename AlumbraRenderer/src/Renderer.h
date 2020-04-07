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
	void drawScene();

private:
	Scene* m_sceneView;
	Shader shader;
};