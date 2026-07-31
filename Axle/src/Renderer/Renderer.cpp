#include "axpch.hpp"

#include "Renderer.hpp"
#include "RenderCommand.hpp"

#include "Renderer/Camera/Camera.hpp"
#include "Renderer/Shaders/Shader.hpp"

namespace Axle {
    Renderer::SceneData* Renderer::s_SceneData = new Renderer::SceneData;

    void Renderer::BeginScene(Camera& camera) {
        s_SceneData->ViewMatrix = camera.GetViewMatrix();
        s_SceneData->ProjectionMatrix = camera.GetProjectionMatrix();
        s_SceneData->ViewProjectionMatrix = s_SceneData->ProjectionMatrix * s_SceneData->ViewMatrix;
    }
    void Renderer::EndScene() {}

    void Renderer::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform) {
        shader->Use();
        shader->SetMat4Uniform("view", s_SceneData->ViewMatrix);
        shader->SetMat4Uniform("projection", s_SceneData->ProjectionMatrix);
        // ShaderManager::SetMat4(shader, "model", transform);

        vertexArray->Bind();
        RenderCommand::DrawElements(vertexArray);
    }
} // namespace Axle
