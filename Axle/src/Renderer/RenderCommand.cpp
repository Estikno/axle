#include "axpch.hpp"

#include <glad/gl.h>

#include "RenderCommand.hpp"
#include "GLDebug.hpp"

#include <glm/glm.hpp>

namespace Axle {
    void RenderCommand::SetClearColor(const glm::vec4& color) {
        AX_GL_CALL(glClearColor(color.r, color.g, color.b, color.a));
    }

    void RenderCommand::Clear() {
        AX_GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
    }

    void RenderCommand::DrawElements(const Ref<VertexArray>& vertexArray) {
        AX_GL_CALL(glDrawElements(GL_TRIANGLES, vertexArray->GetElementBuffer()->GetCount(), GL_UNSIGNED_INT, nullptr));
    }

    void RenderCommand::SetViewport(u32 x, u32 y, u32 width, u32 height) {
        AX_GL_CALL(glViewport(x, y, width, height));
    }
} // namespace Axle
