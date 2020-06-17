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
    const aiScene* scene = importer.ReadFile(path, 
        aiProcess_Triangulate | aiProcess_FlipUVs |aiProcess_CalcTangentSpace);

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
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> tangents;
    std::vector<unsigned int> indices;
    std::vector<MeshTexture> textures;

    // walk through each of the mesh's vertices
    positions.reserve(mesh->mNumVertices);
    normals.reserve(mesh->mNumVertices);
    textures.reserve(mesh->mNumVertices);
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        positions.emplace_back(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        normals.emplace_back(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
        tangents.emplace_back(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
        if (mesh->mTextureCoords[0]) {
            texCoords.emplace_back(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }
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
        std::vector<MeshTexture> normalMaps = loadMaterialTextures(material,
            aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    }

    // return a mesh object created from the extracted mesh data
    return Mesh(positions, normals, texCoords, tangents, indices, textures);
}

/* Checks all material textures of a given type and loads the textures if they're not loaded yet.
    The required info is returned as a Texture struct */
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
