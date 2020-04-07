#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "../Texture.h"

class Model {
public:
	/* Functions */
	Model(const Mesh& mesh);
	/* Constructor, expects a filepath to a 3D model */
	Model(std::string const& path);
	void draw(Shader shader);

private:
	/* Model data */
	std::vector<Mesh> meshes;
	std::string directory;
	// stores all the textures loaded so far
	std::vector<MeshTexture> texturesLoaded;

	/* Functions */
	void loadModel(std::string const& path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<MeshTexture> loadMaterialTextures(aiMaterial* mat, aiTextureType type,
			std::string typeName);
};

