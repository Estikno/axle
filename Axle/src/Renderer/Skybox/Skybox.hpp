#pragma once

#include "axpch.hpp"

#include "Core/Types.hpp"

#include <glm/glm.hpp>

namespace Axle {
    class Skybox {
    public:
        Skybox() = default;
        Skybox(const std::string& texture, const std::string& program);

        ~Skybox();

        Skybox(const Skybox&) = delete;
        Skybox& operator=(const Skybox&) = delete;

        Skybox(Skybox&& other) noexcept;
        Skybox& operator=(Skybox&& other) noexcept;

        void Draw();

        inline void SetViewProjectionMatrix(const glm::mat4& viewProject) {
            m_ViewProjection = viewProject;
        }

    private:
        void Clear();

        u32 m_VAO = 0, m_EBO = 0, m_VBO = 0;

        u32 m_CubemapTexture = 0;
        u32 m_Program = 0, m_VertexShader = 0, m_FragmentShader = 0;

        // Plane information
        std::array<f32, 12> m_Vertices = {
            -1.0f,
            1.0f,
            0.999f,
            -1.0f,
            -1.0f,
            0.999f,
            1.0f,
            -1.0f,
            0.999f,
            1.0f,
            1.0f,
            0.999f,
        };
        std::array<u32, 6> m_Indices = {0, 1, 2, 0, 2, 3};

        glm::mat4 m_ViewProjection = glm::mat4(0.0f);
    };
} // namespace Axle
