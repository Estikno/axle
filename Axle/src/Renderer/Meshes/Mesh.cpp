#include "axpch.hpp"

#include <glad/gl.h>

#include "Mesh.hpp"

#include "Core/Error/Panic.hpp"
#include "Core/Logger/Log.hpp"
#include "Renderer/Textures/Texture.hpp"
#include "Renderer/Primitives/VertexArray.hpp"
#include "Renderer/Primitives/Buffer.hpp"
#include "Renderer/Renderer.hpp"
#include "Renderer/Shaders/Shader.hpp"

#include <tracy/Tracy.hpp>
#include <tracy/TracyOpenGL.hpp>

namespace Axle {
    Mesh::Mesh(const std::vector<Vertex>& vertices,
               const std::vector<u32>& indices,
               std::vector<Ref<Texture2D>>&& textures)
        : m_Vertices(vertices),
          m_Indices(indices),
          m_Textures(std::move(textures)) {
        SetupMesh();
    }

    Mesh::Mesh(Mesh&& other) noexcept
        : m_VAO(std::move(other.m_VAO)),
          m_Vertices(std::move(other.m_Vertices)),
          m_Indices(std::move(other.m_Indices)),
          m_Textures(std::move(other.m_Textures)) {}
    Mesh& Mesh::operator=(Mesh&& other) noexcept {
        if (this != &other) {
            m_VAO = other.m_VAO;

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
        Ref<VertexBuffer> vbuffer =
            Ref<VertexBuffer>::Create(m_Vertices.size() * sizeof(Vertex), reinterpret_cast<f32*>(m_Vertices.data()));

        BufferLayout layout = {{ShaderDataType::Vec3, "position"},
                               {ShaderDataType::Vec3, "normal"},
                               {ShaderDataType::Vec2, "textureCoords"}};
        vbuffer->SetLayout(layout);

        Ref<ElementBuffer> eBuffer = Ref<ElementBuffer>::Create(m_Indices.size(), m_Indices.data());

        m_VAO = Ref<VertexArray>::Create();
        m_VAO->AddVertexBuffer(vbuffer);
        m_VAO->SetIndexBuffer(eBuffer);
    }

    // This basically means how many texture of a specific type can we have
    static constexpr u8 TextureUnitOffset = 3;

    void Mesh::Draw(const Ref<Shader>& shader, const glm::mat4& transform) {
        ZoneScopedN("Draw mesh");
        TracyGpuZone("Draw mesh");

        // These variables can't be larger than TextureUnitOffset
        u8 DiffuseTextureNr = 0;
        u8 SpecularTextureNr = 0;

        // Texture binding
        for (u32 i = 0; i < m_Textures.size(); ++i) {
            switch (m_Textures[i]->GetType()) {
                case TextureType::Diffuse:
                    // TODO: Instead of panicking simply log a warn message
                    AX_ENSURE(DiffuseTextureNr < TextureUnitOffset,
                              LogChannel::Renderer,
                              "Reached maximum number of diffuse textures. Can't bind more");
                    m_Textures[i]->Bind(DiffuseTextureNr + static_cast<u32>(TextureType::Diffuse) * TextureUnitOffset);
                    DiffuseTextureNr++;
                    break;
                case TextureType::Specular:
                    // TODO: Instead of panicking simply log a warn message
                    AX_ENSURE(SpecularTextureNr < TextureUnitOffset,
                              LogChannel::Renderer,
                              "Reached maximum number of specular textures. Can't bind more");
                    m_Textures[i]->Bind(SpecularTextureNr +
                                        static_cast<u32>(TextureType::Specular) * TextureUnitOffset);
                    SpecularTextureNr++;
                    break;
                case TextureType::Unknown:
                    // TODO: Maybe make a special case for unusual textures
                    AX_CORE_WARN(LogChannel::Renderer, "Can't bind a texture with and unknown type");
                    break;
            }
        }

        // Draw the mesh
        Renderer::Submit(shader, m_VAO, transform);
    }
} // namespace Axle
