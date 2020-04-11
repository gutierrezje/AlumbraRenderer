#include "../pch.h"
#include "Model.h"

#include <stb_image.h>

Model::Model(const Mesh& mesh) {
	meshes.push_back(mesh);
}

Model::Model(const std::string& path) {
	loadModel(path);
}

/* Draws the model by drawing all its meshes */
void Model::draw(Shader shader) {
	for (unsigned int i = 0; i < meshes.size(); i++)
		meshes[i].draw(shader);
}

/* Loads a model with supported ASSIMP extensions from file and stores the resulting meshes
	in the meshes vector */
void Model::loadModel(const std::string& path) {
	// read the file via ASSIMP
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	// check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
		return;
	}

	//retrieve the directory path of the filepath
	directory = path.substr(0, path.find_last_of('/'));

	// process ASSIMP's root node recursively
	processNode(scene->mRootNode, scene);
}

/* Processes a node in a recursive fashion. Processes each individual mesh located at the node and
	repeats this process on its children nodes */
void Model::processNode(aiNode* node, const aiScene* scene) {
	// process all the node's meshes (if any)
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}
	// then do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		processNode(node->mChildren[i], scene);
	}
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
	// data to fill
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<MeshTexture> textures;

	// walk through each of the mesh's vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		Vertex vertex;
		// process vertex positions, normals, and texture coordinates
		glm::vec3 vector; // 
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.Position = vector;
		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.Normal = vector;
		if (mesh->mTextureCoords[0]) { // does the mesh contain texture coordinates
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
		}
		else {
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		}

		vertices.push_back(vertex);
	}
	// now walk through each of the mesh's faces and retrieve the corresponding vertex indices
	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		// retrieve all indices of the face and store them int the indices vector
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}
	// process materials
	if (mesh->mMaterialIndex >= 0) {
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<MeshTexture> diffuseMaps = loadMaterialTextures(material,
			aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		std::vector<MeshTexture> specularMaps = loadMaterialTextures(material,
			aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}

	// return a mesh object created from the extracted mesh data
	return Mesh(vertices, indices, textures);
}

/* Checks all material textures of a given type and loads the textures if they're not loaded yet.
	The required info is returned as a TExture struct */
std::vector<MeshTexture> Model::loadMaterialTextures(aiMaterial* mat,
	aiTextureType type, std::string typeName) {
	std::vector<MeshTexture> textures;
	
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
		aiString str;
		mat->GetTexture(type, i, &str);
		// check if the texture was loaded before and if so, continue to next iteration
		bool skip = false;
		for (unsigned int j = 0; j < texturesLoaded.size(); j++) {
			if (std::strcmp(texturesLoaded[j].path.data(), str.C_Str()) == 0) {
				textures.push_back(texturesLoaded[j]);
				skip = true;	// a texture with the same filepath has already been loaded, continue to next
				break;
			}
		}
		
		if (!skip) { // if texture hasn't been loaded already, load it
			MeshTexture texture;
			std::string filename = std::string(str.C_Str());
			filename = directory + '/' + filename;
			Texture tex;
			tex.loadTexture(filename);
			texture.id = tex.textureID();
			texture.type = typeName;
			texture.path = tex.path();
			textures.push_back(texture);
			texturesLoaded.push_back(texture); // add to loaded textures
		}
	}
	return textures;
}
