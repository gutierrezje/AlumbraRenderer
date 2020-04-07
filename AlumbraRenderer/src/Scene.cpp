#include "pch.h"
#include "Scene.h"

Scene::Scene()
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
}
