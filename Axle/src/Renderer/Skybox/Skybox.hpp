#pragma once

#include "axpch.hpp"

#include "Core/Types.hpp"
#include "Renderer/Primitives/VertexArray.hpp"
#include "Renderer/Shaders/Shader.hpp"
#include "Other/CustomTypes/Ref.hpp"
#include "Renderer/Textures/Texture.hpp"

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

        Ref<VertexArray> m_VAO;
        Ref<Shader> m_Shader;

        Ref<TextureCubemap> m_CubemapTexture;

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
