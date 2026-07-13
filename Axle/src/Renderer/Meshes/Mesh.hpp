#pragma once

#include "axpch.hpp"

#include "Core/Types.hpp"
#include "../Textures/Texture.hpp"
#include "../Shaders/ShaderProgram.hpp"

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
             const std::vector<Texture>& textures);

        void Draw(ShaderProgram& program);

    private:
        void SetupMesh();

        u32 m_VAO = 0, m_VBO = 0, m_EBO = 0;

        // Data
        std::vector<Vertex> m_Vertices;
        std::vector<u32> m_Indices;
        std::vector<Texture> m_Textures;
    };
} // namespace Axle
