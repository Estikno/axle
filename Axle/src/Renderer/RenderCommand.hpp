#pragma once

#include "axpch.hpp"

#include "Primitives/VertexArray.hpp"

#include <glm/glm.hpp>

namespace Axle {
    /**
     * OpenGL commands wrap ups
     *
     * All the functionality of this class is NOT THREAD SAFE and should only be accessed by the render thread.
     * */
    class RenderCommand {
    public:
        static void SetClearColor(const glm::vec4& color);
        static void Clear();

        static void DrawElements(const Ref<VertexArray>& vertexArray);

        static void SetViewport(u32 x, u32 y, u32 width, u32 height);

    private:
    };
} // namespace Axle
