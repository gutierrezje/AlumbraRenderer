#include "../pch.h"
#include "Mesh.h"


// Default Constructor
Mesh::Mesh() : VAO(0), VBO(0), EBO(0) {}

// Constructor
Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices,
    std::vector<MeshTexture> textures) {
    this->vertices = vertices;
    this->indices = indices;
    this->textures = textures;

    setupMesh();
}

/* Render the mesh */
void Mesh::draw(Shader shader) {
    // bind appropriate textures
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;

    for (unsigned int i = 0; i < textures.size(); i++) {

        glBindTextureUnit(i, textures[i].id);	// bind proper texture unit before binding
        // retrieve texture number
        std::string number;
        std::string name = textures[i].type;
        if (name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if (name == "texture_specular")
            number = std::to_string(specularNr++);

        // now set the sampler to the correct texture unit
        shader.setSampler(("material." + name + number), i);
    }
    // draw mesh
    glBindVertexArray(VAO);
    if (indices.size() > 0) {
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    }
    else {
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    }
    glBindVertexArray(0);
    glBindTextureUnit(0, 0);
    glBindTextureUnit(1, 0);
}

/* Initializes all the buffer objects/arrays */
void Mesh::setupMesh() {
    // create buffers/arrays
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex),
        &vertices[0], GL_STATIC_DRAW);

    if (indices.size() > 0) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
            &indices[0], GL_STATIC_DRAW);
    }

    // set the vertex attribute pointers
    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (void*)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
}
