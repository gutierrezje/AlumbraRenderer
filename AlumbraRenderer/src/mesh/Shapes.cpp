#include "../pch.h"
#include "Shapes.h"

Cube::Cube(const std::string& diffTexPath, const std::string& specTexPath, const std::string& normTexPath)
{
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
    else {
        // Use the same as diffuse texture
        m_textures.push_back({ texture.textureID(), "texture_specular", texture.path() });
    }

    if (normTexPath != "") {
        texture.loadTexture(normTexPath);
        m_textures.push_back({ texture.textureID(), "texture_normal", texture.path() });
        /*
        for (auto i = 0ll; i < m_positions.size(); i += 3ll) {
            glm::vec3 edge1 = m_positions[i + 1ll] - m_positions[i];
            glm::vec3 edge2 = m_positions[i + 2ll] - m_positions[i];

            glm::vec2 deltaUV1 = m_texCoords[i + 1ll] - m_texCoords[i];
            glm::vec2 deltaUV2 = m_texCoords[i + 2ll] - m_texCoords[i];

            float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

            glm::vec3 tangent = f * (deltaUV2.y * edge1 - deltaUV1.y * edge2);
            m_tangents.push_back(tangent);
            m_tangents.push_back(tangent);
            m_tangents.push_back(tangent);
        }
        */
    }
    else {
        m_textures.push_back({ 0, "texture_null", "" });
    }

    setupMesh();
}

Cube::~Cube() {}

Quad::Quad(const std::string& diffTexPath, const std::string& specTexPath, const std::string& normTexPath)
{
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
    else {
        // Use the same as diffuse texture
        m_textures.push_back({ texture.textureID(), "texture_specular", texture.path() });
    }

    if (normTexPath != "") {
        texture.loadTexture(normTexPath);
        m_textures.push_back({ texture.textureID(), "texture_normal", texture.path() });
    }
    else {
        m_textures.push_back({ 0, "texture_null", "" });
    }

    setupMesh();
}

Quad::~Quad() {}