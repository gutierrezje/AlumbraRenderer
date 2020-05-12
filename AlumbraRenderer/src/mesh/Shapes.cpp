#include "../pch.h"
#include "Shapes.h"

Cube::Cube(const std::string& diffTexPath, const std::string& specTexPath)
{
    //vertices = cubeVertices;
    m_positions = cubePositions;
    m_normals = cubeNormals;
    m_texCoords = cubeTexCoords;
    Texture texture;
    texture.loadTexture(diffTexPath);
    m_textures.push_back({ texture.textureID(), "texture_diffuse", texture.path() });

    if (specTexPath != "") {
        texture.loadTexture(specTexPath);
        m_textures.push_back({ texture.textureID(), "texture_specular", texture.path() });
    }

    setupMesh();
}

Cube::~Cube() {}

Quad::Quad(const std::string& diffTexPath, const std::string& specTexPath)
{
    //vertices = quadVertices;
    m_positions = quadPositions;
    m_normals = quadNormals;
    m_texCoords = quadTexCoords;
    Texture texture;
    texture.loadTexture(diffTexPath);
    m_textures.push_back({ texture.textureID(), "texture_diffuse", texture.path() });

    if (specTexPath != "") {
        texture.loadTexture(specTexPath);
        m_textures.push_back({ texture.textureID(), "texture_specular", texture.path() });
    }

    setupMesh();
}

Quad::~Quad() {}