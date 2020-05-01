#include "../pch.h"
#include "Shapes.h"

Cube::Cube(const std::string& diffTexPath, const std::string& specTexPath)
{
    vertices = cubeVertices;
    Texture texture;
    texture.loadTexture(diffTexPath);
    textures.push_back({ texture.textureID(), "texture_diffuse", texture.path() });

    if (specTexPath != "") {
        texture.loadTexture(specTexPath);
        textures.push_back({ texture.textureID(), "texture_specular", texture.path() });
    }

    setupMesh();
}

Cube::~Cube() {}

Quad::Quad(const std::string& diffTexPath, const std::string& specTexPath)
{
    vertices = quadVertices;
    Texture texture;
    texture.loadTexture(diffTexPath);
    textures.push_back({ texture.textureID(), "texture_diffuse", texture.path() });

    if (specTexPath != "") {
        texture.loadTexture(specTexPath);
        textures.push_back({ texture.textureID(), "texture_specular", texture.path() });
    }

    setupMesh();
}

Quad::~Quad() {}