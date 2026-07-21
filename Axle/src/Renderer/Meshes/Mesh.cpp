#include "axpch.hpp"

#include <glad/gl.h>

#include "Mesh.hpp"

#include "Core/Error/Panic.hpp"
#include "Core/Logger/Log.hpp"
#include "Renderer/Textures/Texture.hpp"
#include "Renderer/Textures/TextureManager.hpp"

#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

namespace Axle {
    Mesh::Mesh(const std::vector<Vertex>& vertices,
               const std::vector<u32>& indices,
               std::vector<std::pair<u32, TextureType>>&& textures)
        : m_Vertices(vertices),
          m_Indices(indices),
          m_Textures(std::move(textures)) {
        SetupMesh();
    }

    Mesh::~Mesh() {
        TracyGpuZone("Delete mesh");

        if (m_VAO != 0)
            glDeleteVertexArrays(1, &m_VAO);
        if (m_EBO != 0)
            glDeleteBuffers(1, &m_EBO);
        if (m_VBO != 0)
            glDeleteBuffers(1, &m_VBO);
    }

    Mesh::Mesh(Mesh&& other) noexcept
        : m_VAO(other.m_VAO),
          m_EBO(other.m_EBO),
          m_VBO(other.m_VBO),
          m_Vertices(std::move(other.m_Vertices)),
          m_Indices(std::move(other.m_Indices)),
          m_Textures(std::move(other.m_Textures)) {
        other.m_VAO = 0;
        other.m_VBO = 0;
        other.m_EBO = 0;
    }
    Mesh& Mesh::operator=(Mesh&& other) noexcept {
        if (this != &other) {
            if (m_VAO != 0)
                glDeleteVertexArrays(1, &m_VAO);
            if (m_EBO != 0)
                glDeleteBuffers(1, &m_EBO);
            if (m_VBO != 0)
                glDeleteBuffers(1, &m_VBO);

            m_VAO = other.m_VAO;
            m_VBO = other.m_VBO;
            m_EBO = other.m_EBO;

            other.m_VAO = 0;
            other.m_VBO = 0;
            other.m_EBO = 0;

            m_Vertices = std::move(other.m_Vertices);
            m_Indices = std::move(other.m_Indices);
            m_Textures = std::move(other.m_Textures);
        }
        return *this;
    }

    void Mesh::SetupMesh() {
        ZoneScopedN("SetupMesh");
        TracyGpuZone("SetupMesh");

        // Create buffers
        glCreateVertexArrays(1, &m_VAO);
        glCreateBuffers(1, &m_VBO);
        glCreateBuffers(1, &m_EBO);

        // Store data
        glNamedBufferData(m_VBO, sizeof(Vertex) * m_Vertices.size(), m_Vertices.data(), GL_STATIC_DRAW);
        glVertexArrayVertexBuffer(m_VAO, 0, m_VBO, 0, sizeof(Vertex));
        glNamedBufferData(m_EBO, sizeof(u32) * m_Indices.size(), m_Indices.data(), GL_STATIC_DRAW);
        glVertexArrayElementBuffer(m_VAO, m_EBO);

        // Vertex positions
        glEnableVertexArrayAttrib(m_VAO, 0);
        glVertexArrayAttribBinding(m_VAO, 0, 0);
        glVertexArrayAttribFormat(m_VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);

        // Vertex normals
        glEnableVertexArrayAttrib(m_VAO, 1);
        glVertexArrayAttribBinding(m_VAO, 1, 0);
        glVertexArrayAttribFormat(m_VAO, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));

        // Vertex texture coordinates
        glEnableVertexArrayAttrib(m_VAO, 2);
        glVertexArrayAttribBinding(m_VAO, 2, 0);
        glVertexArrayAttribFormat(m_VAO, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, textureCoords));
    }

    // This basically means how many texture of a specific type can we have
    static constexpr u8 TextureUnitOffset = 3;

    void Mesh::Draw(u32 program) {
        ZoneScopedN("Draw mesh");
        TracyGpuZone("Draw mesh");

        // These variables can't be larger than TextureUnitOffset
        u8 DiffuseTextureNr = 0;
        u8 SpecularTextureNr = 0;

        // Texture binding
        for (u32 i = 0; i < m_Textures.size(); ++i) {
            switch (m_Textures[i].second) {
                case TextureType::Diffuse:
                    // TODO: Instead of panicking simply log a warn message
                    AX_ENSURE(DiffuseTextureNr < TextureUnitOffset,
                              LogChannel::Renderer,
                              "Reached maximum number of diffuse textures. Can't bind more");
                    TextureManager::Bind(m_Textures[i].first,
                                         DiffuseTextureNr + static_cast<u8>(TextureType::Diffuse) * TextureUnitOffset);
                    DiffuseTextureNr++;
                    break;
                case TextureType::Specular:
                    // TODO: Instead of panicking simply log a warn message
                    AX_ENSURE(SpecularTextureNr < TextureUnitOffset,
                              LogChannel::Renderer,
                              "Reached maximum number of specular textures. Can't bind more");
                    TextureManager::Bind(m_Textures[i].first,
                                         SpecularTextureNr +
                                             static_cast<u8>(TextureType::Specular) * TextureUnitOffset);
                    SpecularTextureNr++;
                    break;
                case TextureType::Unknown:
                    // TODO: Maybe make a special case for unusual textures
                    AX_CORE_WARN(LogChannel::Renderer, "Can't bind a texture with and unknown type");
                    break;
            }
        }

        // Draw the mesh
        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_INT, 0);
    }
} // namespace Axle
