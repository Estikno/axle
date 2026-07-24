#include "axpch.hpp"

#include "Renderer.hpp"
#include "RenderCommand.hpp"

namespace Axle {
    void Renderer::BeginScene() {}
    void Renderer::EndScene() {}

    void Renderer::Submit(const Ref<VertexArray>& vertexArray) {
        vertexArray->Bind();
        RenderCommand::DrawElements(vertexArray);
    }
} // namespace Axle
