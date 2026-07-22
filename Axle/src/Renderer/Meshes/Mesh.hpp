#pragma once

#include "axpch.hpp"

#include "Core/Types.hpp"
#include "../Textures/Texture.hpp"

#include <glm/glm.hpp>

namespace Axle {
    struct Vertex {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 textureCoords;
    };

    class Mesh {
    public:
        Mesh(const std::vector<Vertex>& vertices,
             const std::vector<u32>& indices,
             std::vector<std::pair<u32, TextureType>>&& textures);
        ~Mesh();

        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;

        Mesh(Mesh&& other) noexcept;
        Mesh& operator=(Mesh&& other) noexcept;

        void Draw(u32 program);

    private:
        void SetupMesh();
        void Clear();

        u32 m_VAO = 0, m_VBO = 0, m_EBO = 0;

        // Data
        std::vector<Vertex> m_Vertices;
        std::vector<u32> m_Indices;
        std::vector<std::pair<u32, TextureType>> m_Textures;
    };
} // namespace Axle
