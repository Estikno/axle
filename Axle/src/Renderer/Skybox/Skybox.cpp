#include "axpch.hpp"

#include <glad/gl.h>

#include "Skybox.hpp"
#include "Renderer/Shaders/Shader.hpp"
#include "Renderer/GLDebug.hpp"
#include "Renderer/Primitives/Buffer.hpp"
#include "Renderer/Primitives/VertexArray.hpp"
#include "Renderer/Textures/Texture.hpp"

#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

namespace Axle {
    Skybox::Skybox(const std::string& texture, const std::string& program) {
        ZoneScopedN("Setup Skybox");
        TracyGpuZone("Setup Skybox");

        // Setup buffers
        Ref<VertexBuffer> vBuffer = Ref<VertexBuffer>::Create(sizeof(f32) * m_Vertices.size(), m_Vertices.data());
        Ref<ElementBuffer> eBuffer = Ref<ElementBuffer>::Create(m_Indices.size(), m_Indices.data());
        m_VAO = Ref<VertexArray>::Create();

        BufferLayout layout = {{ShaderDataType::Vec3, "aPos"}};
        vBuffer->SetLayout(layout);

        m_VAO->AddVertexBuffer(vBuffer);
        m_VAO->SetIndexBuffer(eBuffer);

        // Setup texture and program
        m_CubemapTexture = TextureCubemap::Create(texture);
        m_Shader = Shader::Create(program);
    }

    Skybox::~Skybox() {
        Clear();
    }

    Skybox::Skybox(Skybox&& other) noexcept
        : m_VAO(std::move(other.m_VAO)),
          m_CubemapTexture(other.m_CubemapTexture),
          m_Shader(std::move(other.m_Shader)) {}

    Skybox& Skybox::operator=(Skybox&& other) noexcept {
        if (this != &other) {
            Clear();

            m_VAO = std::move(other.m_VAO);
            m_CubemapTexture = other.m_CubemapTexture;
            m_Shader = std::move(other.m_Shader);
        }
        return *this;
    }

    void Skybox::Draw() {
        ZoneScopedN("Draw Skybox");
        TracyGpuZone("Draw Skybox");

        m_Shader->Use();
        m_VAO->Bind();

        m_Shader->SetMat4Uniform("viewProjection", m_ViewProjection);
        m_CubemapTexture->Bind(0);

        AX_GL_CALL(glDepthMask(GL_FALSE));
        AX_GL_CALL(glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_INT, 0));
        AX_GL_CALL(glDepthMask(GL_TRUE));
    }

    void Skybox::Clear() {
        TracyGpuZone("Delete Skybox");

        m_VAO.Reset();
        m_Shader.Reset();
    }
} // namespace Axle
