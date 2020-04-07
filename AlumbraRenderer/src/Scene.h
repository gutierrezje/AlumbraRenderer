#pragma once

#include "mesh/Model.h"
#include "mesh/Shapes.h"

/**
 * Manages aspects of the scene that the renderer will draw
 */
class Scene
{
public:
	Scene();
	~Scene();

	void init();
	inline std::vector<Model*>& models() { return m_models; }
private:
	std::vector<Model*> m_models;
};
