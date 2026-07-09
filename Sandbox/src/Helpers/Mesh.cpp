#include <glad/gl.h>

#include "Mesh.hpp"
#include <cstddef>

using namespace Axle;

void Mesh::SetupMesh() {
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

    glBufferData(GL_ARRAY_BUFFER, p_Vertices.size() * sizeof(Vertex), p_Vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, p_Indices.size() * sizeof(u32), p_Indices.data(), GL_STATIC_DRAW);

    // Vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, Position));
    // Vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, Normal));
    // Vertex positions
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(Vertex, TexCoords));

    glBindVertexArray(0);
}

void Mesh::Draw(ShaderHelper& shader) {
    u32 diffuseNr = 1;
    u32 specularNr = 1;

    for (u32 i = 0; i < p_Textures.size(); ++i) {
        glActiveTexture(GL_TEXTURE0 + i);

        std::string number;
        std::string name = p_Textures[i].type;

        if (name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if (name == "texture_specular")
            number = std::to_string(specularNr++);

        shader.SetInt("material." + name + number, i);
        glBindTexture(GL_TEXTURE_2D, p_Textures[i].id);
    }

    glActiveTexture(GL_TEXTURE0);

    // Draw mesh
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, p_Indices.size(), GL_UNSIGNED_INT, 0);
}
