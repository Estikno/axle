#include "axpch.hpp"

#include <glad/gl.h>

#include "Skybox.hpp"
#include "Renderer/Shaders/Shader.hpp"
#include "Renderer/GLDebug.hpp"
#include "Renderer/Primitives/VertexArray.hpp"
#include "Renderer/Textures/Texture.hpp"
#include "Renderer/Renderer.hpp"

#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

namespace Axle {
    Skybox::Skybox(const std::string& texture, const std::string& program) {
        ZoneScopedN("Setup Skybox");
        TracyGpuZone("Setup Skybox");

        // Setup buffer
        m_VAO = VertexArray::ScreenQuad();

        // Setup texture and program
        m_CubemapTexture = TextureCubemap::Create(texture);
        m_Shader = Shader::Create(program);
    }

    Skybox::~Skybox() {
        Reset();
    }

    Skybox::Skybox(Skybox&& other) noexcept
        : m_VAO(std::move(other.m_VAO)),
          m_CubemapTexture(other.m_CubemapTexture),
          m_Shader(std::move(other.m_Shader)) {}

    Skybox& Skybox::operator=(Skybox&& other) noexcept {
        if (this != &other) {
            Reset();

            m_VAO = std::move(other.m_VAO);
            m_CubemapTexture = other.m_CubemapTexture;
            m_Shader = std::move(other.m_Shader);
        }
        return *this;
    }

    void Skybox::Draw() {
        ZoneScopedN("Draw Skybox");
        TracyGpuZone("Draw Skybox");

        m_CubemapTexture->Bind(0);

        AX_GL_CALL(glDepthMask(GL_FALSE));
        Renderer::Submit(m_Shader, m_VAO);
        AX_GL_CALL(glDepthMask(GL_TRUE));
    }

    void Skybox::Reset() {
        TracyGpuZone("Delete Skybox");

        m_VAO.Reset();
        m_Shader.Reset();
    }
} // namespace Axle
