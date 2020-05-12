#include "../pch.h"
#include "Mesh.h"
#include "../Buffers.h"

// Default Constructor
Mesh::Mesh() {}

Mesh::Mesh(std::vector<float>& positions, std::vector<float>& normals,
    std::vector<float>& texCoords, std::vector<unsigned int>& indices,
    std::vector<MeshTexture>& textures)
{
    m_positions = positions;
    m_normals = normals;
    m_texCoords = texCoords;
    this->m_indices = indices;
    this->m_textures = textures;

    setupMesh();
}

/* Render the mesh */
void Mesh::draw(Shader shader)
{
    // bind appropriate textures
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;

    for (unsigned int i = 0; i < m_textures.size(); i++) {
        // bind proper texture unit before binding
        glBindTextureUnit(i, m_textures[i].id);	
        // retrieve texture number
        std::string number;
        std::string name = m_textures[i].type;
        if (name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if (name == "texture_specular")
            number = std::to_string(specularNr++);

        // now set the sampler to the correct texture unit
        shader.setSampler(("material." + name + number), i);
    }
    // draw mesh
    glBindVertexArray(VAO);
    if (m_indices.size() > 0) {
        glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
    }
    else {
        glDrawArrays(GL_TRIANGLES, 0, m_positions.size() / 3);
    }

    glBindTextureUnit(1, 0);
}

/* Initializes all the buffer objects/arrays */
void Mesh::setupMesh()
{
    auto bufferSize = sizeof(unsigned int) * m_indices.size()
        + sizeof(float) * (m_positions.size()
        + m_normals.size() + m_texCoords.size());
    DataBuffer buffer(bufferSize, m_positions.size() / 3, 3, m_indices.size());
    buffer.addIndices(m_indices.data());
    buffer.addData(m_positions.data(), 3);
    buffer.addData(m_normals.data(), 3);
    buffer.addData(m_texCoords.data(), 2);
    VertexArray vao;
    vao.loadBuffer(buffer, 2);
    VAO = vao.vertexArrayID();
}
