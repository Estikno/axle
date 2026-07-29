#include "axpch.hpp"

#include "Renderer.hpp"
#include "RenderCommand.hpp"

#include "Renderer/Camera/Camera.hpp"
#include "Renderer/Shaders/ShaderManager.hpp"

namespace Axle {
    Renderer::SceneData* Renderer::s_SceneData = new Renderer::SceneData;

    void Renderer::BeginScene(Camera& camera) {
        s_SceneData->ViewMatrix = camera.GetViewMatrix();
        s_SceneData->ProjectionMatrix = camera.GetProjectionMatrix();
        s_SceneData->ViewProjectionMatrix = s_SceneData->ProjectionMatrix * s_SceneData->ViewMatrix;
    }
    void Renderer::EndScene() {}

    void Renderer::Submit(const Ref<VertexArray>& vertexArray, u32 shader) {
        ShaderManager::UseProgram(shader);
        ShaderManager::SetMat4(shader, "view", s_SceneData->ViewMatrix);
        ShaderManager::SetMat4(shader, "projection", s_SceneData->ProjectionMatrix);

        vertexArray->Bind();
        RenderCommand::DrawElements(vertexArray);
    }
} // namespace Axle
