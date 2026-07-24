#pragma once

#include "axpch.hpp"

#include "Primitives/VertexArray.hpp"

#include <glm/glm.hpp>

namespace Axle {
    class RenderCommand {
    public:
        static void SetClearColor(const glm::vec4& color);
        static void Clear();

        static void DrawElements(const Ref<VertexArray>& vertexArray);

    private:
    };
} // namespace Axle
