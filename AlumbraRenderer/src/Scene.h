#pragma once

#include "mesh/Model.h"
#include "mesh/Shapes.h"
#include "Cubemap.h"

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
	inline Cubemap& cubemap() { return m_cubemap; }
private:
	std::vector<Model*> m_models;
	Cubemap m_cubemap;
};
