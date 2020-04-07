#pragma once

#include "../Shader.h"

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

struct MeshTexture {
	unsigned int id;
	std::string type;
	std::string path;
};

class Mesh {
public:
	/* Mesh data */
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<MeshTexture> textures;
	/* Functions */
	Mesh();
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, 
			std::vector<MeshTexture> textures);
	void draw(Shader shader);
protected:
	/* Render data */
	unsigned int VAO, VBO, EBO;

	/* Functions */
	void setupMesh();
};
