#include "axpch.hpp"

#include <glad/gl.h>

#include "Skybox.hpp"
#include "Renderer/Textures/TextureManager.hpp"
#include "Renderer/Shaders/ShaderManager.hpp"
#include "Renderer/GLDebug.hpp"

#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

namespace Axle {
    Skybox::Skybox(const std::string& texture, const std::string& program) {
        ZoneScopedN("Setup Skybox");
        TracyGpuZone("Setup Skybox");

        // Setup buffers
        AX_GL_CALL(glCreateVertexArrays(1, &m_VAO));
        AX_GL_CALL(glCreateBuffers(1, &m_VBO));
        AX_GL_CALL(glCreateBuffers(1, &m_EBO));

        AX_GL_CALL(glNamedBufferData(m_VBO, sizeof(f32) * m_Vertices.size(), m_Vertices.data(), GL_STATIC_DRAW));
        AX_GL_CALL(glVertexArrayVertexBuffer(m_VAO, 0, m_VBO, 0, sizeof(f32) * 3));
        AX_GL_CALL(glNamedBufferData(m_EBO, sizeof(u32) * m_Indices.size(), m_Indices.data(), GL_STATIC_DRAW));
        AX_GL_CALL(glVertexArrayElementBuffer(m_VAO, m_EBO));

        AX_GL_CALL(glEnableVertexArrayAttrib(m_VAO, 0));
        AX_GL_CALL(glVertexArrayAttribBinding(m_VAO, 0, 0));
        AX_GL_CALL(glVertexArrayAttribFormat(m_VAO, 0, 3, GL_FLOAT, GL_FALSE, 0));

        // Setup texture and program
        m_CubemapTexture = TextureManager::CreateCubeMap(texture);
        m_VertexShader = ShaderManager::CreateShader(program, ShaderType::Vertex);
        m_FragmentShader = ShaderManager::CreateShader(program, ShaderType::Fragment);
        m_Program = ShaderManager::CreateShaderProgram(m_VertexShader, m_FragmentShader);
    }

    Skybox::~Skybox() {
        Clear();
    }

    Skybox::Skybox(Skybox&& other) noexcept
        : m_VAO(other.m_VAO),
          m_EBO(other.m_EBO),
          m_VBO(other.m_VBO),
          m_CubemapTexture(other.m_CubemapTexture),
          m_Program(other.m_Program),
          m_VertexShader(other.m_VertexShader),
          m_FragmentShader(other.m_FragmentShader) {
        other.m_VAO = 0;
        other.m_EBO = 0;
        other.m_VBO = 0;
    }

    Skybox& Skybox::operator=(Skybox&& other) noexcept {
        if (this != &other) {
            Clear();

            m_VAO = other.m_VAO;
            m_EBO = other.m_EBO;
            m_VBO = other.m_VBO;
            m_CubemapTexture = other.m_CubemapTexture;
            m_Program = other.m_Program;
            m_VertexShader = other.m_VertexShader;
            m_FragmentShader = other.m_FragmentShader;

            other.m_VAO = 0;
            other.m_EBO = 0;
            other.m_VBO = 0;
        }
        return *this;
    }

    void Skybox::Draw() {
        ZoneScopedN("Draw Skybox");
        TracyGpuZone("Draw Skybox");

        AX_GL_CALL(glUseProgram(m_Program));
        AX_GL_CALL(glBindVertexArray(m_VAO));

        ShaderManager::SetMat4(m_Program, "viewProjection", m_ViewProjection);
        TextureManager::Bind(m_CubemapTexture, 0);

        AX_GL_CALL(glDepthMask(GL_FALSE));
        AX_GL_CALL(glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_INT, 0));
        AX_GL_CALL(glDepthMask(GL_TRUE));
    }

    void Skybox::Clear() {
        TracyGpuZone("Delete Skybox");

        if (m_VAO != 0) {
            AX_GL_CALL(glDeleteVertexArrays(1, &m_VAO));
        }
        if (m_VBO != 0) {
            AX_GL_CALL(glDeleteBuffers(1, &m_VBO));
        }
        if (m_EBO != 0) {
            AX_GL_CALL(glDeleteBuffers(1, &m_EBO));
        }
    }
} // namespace Axle
