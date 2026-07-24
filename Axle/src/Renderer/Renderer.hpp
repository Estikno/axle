#pragma once

#include "axpch.hpp"

#include "Renderer/Primitives/VertexArray.hpp"

namespace Axle {
    class Renderer {
    public:
        static void BeginScene();
        static void EndScene();

        static void Submit(const Ref<VertexArray>& vertexArray);

    private:
    };
} // namespace Axle
