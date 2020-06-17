#include "../pch.h"
#include "Mesh.h"
#include "../Buffers.h"

// Default Constructor
Mesh::Mesh() {}

Mesh::Mesh(std::vector<glm::vec3>& positions, std::vector<glm::vec3>& normals,
    std::vector<glm::vec2>& texCoords, std::vector<glm::vec3>& tangents,
    std::vector<unsigned int>& indices, std::vector<MeshTexture>& textures)
{
    m_positions = positions;
    m_normals = normals;
    m_texCoords = texCoords;
    m_tangents = tangents;
    m_indices = indices;
    m_textures = textures;

    setupMesh();
}

/* Render the mesh */
void Mesh::draw(Shader shader)
{
    // bind appropriate textures
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1u;

    shader.setBool("useNormalMap", false);
    for (unsigned int i = 0; i < m_textures.size(); i++) {
        // retrieve texture number
        std::string number;
        std::string name = m_textures[i].type;
        if (name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if (name == "texture_specular")
            number = std::to_string(specularNr++);
        else if (name == "texture_normal") {
            number = std::to_string(normalNr++);
            shader.setBool("useNormalMap", true);
        }
        else
            continue;

        // now set the sampler to the correct texture unit
        shader.setSampler(("material." + name + number), i);
        // bind proper texture unit after binding
        glBindTextureUnit(i, m_textures[i].id);
    }
    // draw mesh
    glBindVertexArray(m_meshVAO);
    if (m_indices.size() > 0) {
        glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
    }
    else {
        glDrawArrays(GL_TRIANGLES, 0, m_positions.size());
    }

    glBindTextureUnit(1, 0);
    glBindTextureUnit(2, 0);
}

/* Initializes all the buffer objects/arrays */
void Mesh::setupMesh()
{
    
    auto bufferSize = sizeof(unsigned int) * m_indices.size()
        + sizeof(m_positions[0]) * (m_positions.size() + m_normals.size() + m_tangents.size() + m_bitangents.size())
        + sizeof(m_texCoords[0]) * m_texCoords.size();
    DataBuffer buffer(bufferSize, m_positions.size(), 4, m_indices.size());
    buffer.addIndices(m_indices.data());
    buffer.addVec3s(m_positions.data());
    buffer.addVec3s(m_normals.data());
    buffer.addVec2s(m_texCoords.data());
    buffer.addVec3s(m_tangents.data());
    //buffer.addVec3s(m_bitangents.data());
    VertexArray vao;
    vao.loadBuffer(buffer, 2);
    m_meshVAO = vao.vertexArrayID();
}
