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
    std::vector<float> m_positions;
    std::vector<float> m_normals;
    std::vector<float> m_texCoords;
    std::vector<unsigned int> m_indices;
    std::vector<MeshTexture> m_textures;
    int m_vertexCount;
    int m_indexCount;
    /* Functions */
    Mesh();
    Mesh(std::vector<float>& positions, std::vector<float>& normals,
        std::vector<float>& texCoords, std::vector<unsigned int>& indices,
        std::vector<MeshTexture>& textures);
    void draw(Shader shader);
protected:
    /* Render data */
    unsigned int VAO;

    /* Functions */
    void setupMesh();
};
