#pragma once

#include "../Shader.h"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct MeshTexture {
    GLuint id;
    std::string type;
    std::string path;
};

class Mesh {
public:
    /* Mesh data */
    std::vector<glm::vec3> m_positions;
    std::vector<glm::vec3> m_normals;
    std::vector<glm::vec2> m_texCoords;
    std::vector<glm::vec3> m_tangents;
    std::vector<glm::vec3> m_bitangents;
    std::vector<unsigned int> m_indices;
    std::vector<MeshTexture> m_textures;
    /* Functions */
    Mesh();
    Mesh(std::vector<glm::vec3>& positions, std::vector<glm::vec3>& normals,
        std::vector<glm::vec2>& texCoords, std::vector<glm::vec3>& tangents,
        std::vector<glm::vec3>& bitangents, std::vector<unsigned int>& indices,
        std::vector<MeshTexture>& textures);
    void draw(Shader shader);
protected:
    /* Render data */
    unsigned int m_meshVAO;

    /* Functions */
    void setupMesh();
};
