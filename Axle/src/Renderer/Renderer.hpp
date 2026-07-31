#pragma once

#include "axpch.hpp"

#include "Renderer/Primitives/VertexArray.hpp"
#include "Renderer/Camera/Camera.hpp"
#include "Renderer/Shaders/Shader.hpp"

#include "glm/fwd.hpp"

namespace Axle {
    class Renderer {
    public:
        static void BeginScene(Camera& camera);
        static void EndScene();

        static void Submit(const Ref<Shader>& shader,
                           const Ref<VertexArray>& vertexArray,
                           const glm::mat4& transform = glm::mat4(1.0f));

    private:
        struct SceneData {
            glm::mat4 ViewProjectionMatrix;
            glm::mat4 ViewMatrix;
            glm::mat4 ProjectionMatrix;
        };

        static SceneData* s_SceneData;
    };
} // namespace Axle
