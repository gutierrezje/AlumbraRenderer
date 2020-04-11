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
