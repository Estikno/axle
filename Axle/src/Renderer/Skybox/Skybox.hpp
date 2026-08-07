#pragma once

#include "axpch.hpp"

#include "Core/Types.hpp"
#include "Renderer/Primitives/VertexArray.hpp"
#include "Renderer/Shaders/Shader.hpp"
#include "Other/CustomTypes/Ref.hpp"
#include "Renderer/Textures/Texture.hpp"

#include <glm/glm.hpp>

namespace Axle {
    class Skybox : public RefCounted {
    public:
        Skybox() = default;
        Skybox(const std::string& texture, const std::string& program);

        ~Skybox();

        Skybox(const Skybox&) = delete;
        Skybox& operator=(const Skybox&) = delete;

        Skybox(Skybox&& other) noexcept;
        Skybox& operator=(Skybox&& other) noexcept;

        void Draw();

    private:
        void Reset();

        Ref<VertexArray> m_VAO;
        Ref<Shader> m_Shader;

        Ref<TextureCubemap> m_CubemapTexture;
    };
} // namespace Axle
