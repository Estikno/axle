#include "axpch.hpp"

#include <glad/gl.h>
#include "GLDebug.hpp"

#include "Renderer.hpp"
#include "RenderCommand.hpp"

#include "Renderer/Camera/Camera.hpp"
#include "Renderer/Shaders/ShaderManager.hpp"
#include "Renderer/Textures/TextureManager.hpp"
#include "Renderer/Primitives/FrameBuffer.hpp"
#include "Renderer/Primitives/VertexArray.hpp"
#include "Renderer/Skybox/Skybox.hpp"
#include "Renderer/Shaders/Shader.hpp"
#include "Other/CustomTypes/Ref.hpp"
#include "Core/Error/Panic.hpp"
#include "Core/Logger/Log.hpp"

namespace Axle {
    std::vector<SceneData> Renderer::s_SceneData;
    u32 Renderer::s_UBO;
    Ref<VertexArray> Renderer::s_DTextureVAO;
    Ref<Shader> Renderer::s_TexShader;

    void Renderer::Init() {
        ShaderManager::Init();
        TextureManager::Init();

        AX_GL_CALL(glCreateBuffers(1, &s_UBO));
        AX_GL_CALL(glNamedBufferData(s_UBO, sizeof(ScenePOD), nullptr, GL_DYNAMIC_DRAW));

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

        Ref<VertexBuffer> vBuffer = Ref<VertexBuffer>::Create(sizeof(f32) * m_Vertices.size(), m_Vertices.data());
        Ref<ElementBuffer> eBuffer = Ref<ElementBuffer>::Create(m_Indices.size(), m_Indices.data());
        s_DTextureVAO = Ref<VertexArray>::Create();

        BufferLayout layout = {{ShaderDataType::Vec3, "aPos"}};
        vBuffer->SetLayout(layout);

        s_DTextureVAO->AddVertexBuffer(vBuffer);
        s_DTextureVAO->SetIndexBuffer(eBuffer);

        s_TexShader = Shader::Create("Sandbox/src/Shaders/textureDraw.bin");
    }

    void Renderer::Shutdown() {
        s_TexShader.Reset();
        s_DTextureVAO.Reset();
        AX_GL_CALL(glDeleteBuffers(1, &s_UBO));

        TextureManager::Shutdown();
        ShaderManager::Shutdown();
    }

    SceneHandle Renderer::BeginScene(Camera& camera, const Ref<Skybox>& skybox, const Ref<FrameBuffer>& target) {
        SceneData data{};

        data.ViewMatrix = camera.GetViewMatrix();
        data.ProjectionMatrix = camera.GetProjectionMatrix();
        data.ViewProjectionMatrix = data.ProjectionMatrix * data.ViewMatrix;
        data.CameraPosition = camera.GetPosition();

        data.SkyboxScene = skybox;

        data.RenderTarget = target;

        s_SceneData.push_back(data);
        u32 index = static_cast<u32>(s_SceneData.size()) - 1;

        BindSceneState(s_SceneData.back());

        RenderCommand::SetClearColor(glm::vec4(0.2f, 0.3f, 0.3f, 1.0f));
        RenderCommand::Clear();

        return SceneHandle(&s_SceneData.back(), index);
    }

    void Renderer::EndScene(SceneHandle& handle) {
        AX_ASSERT(
            handle.StackIndex == s_SceneData.size() - 1, LogChannel::Renderer, "Scenes must end in strict LIFO order");

        if (handle.Data->SkyboxScene)
            handle.Data->SkyboxScene->Draw();

        s_SceneData.pop_back();

        if (!s_SceneData.empty())
            BindSceneState(s_SceneData.back());
    }

    void Renderer::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform) {
        shader->Use();
        shader->SetMat4Uniform("u_Model", transform);

        vertexArray->Bind();
        RenderCommand::DrawElements(vertexArray);
    }

    void Renderer::Submit(const Ref<Texture2D>& texture) {
        s_TexShader->Use();
        s_DTextureVAO->Bind();

        RenderCommand::DrawElements(s_DTextureVAO);
    }

    void Renderer::OnFrameBufferResize(u32 widht, u32 height) {
        RenderCommand::SetViewport(0, 0, widht, height);
    }

    void Renderer::BindSceneState(SceneData& data) {
        // Update UBO
        ScenePOD podData(data);
        AX_GL_CALL(glNamedBufferSubData(s_UBO, 0, sizeof(ScenePOD), &podData));
        AX_GL_CALL(glBindBufferBase(GL_UNIFORM_BUFFER, 0, s_UBO));

        // Bind FrameBuffer
        if (data.RenderTarget)
            data.RenderTarget->Bind();
        else
            FrameBuffer::BindDefault();
    }
} // namespace Axle
