#pragma once

#include "axpch.hpp"

#include "Renderer/Primitives/VertexArray.hpp"
#include "Renderer/Camera/Camera.hpp"
#include "Renderer/Shaders/Shader.hpp"

#include "glm/fwd.hpp"

namespace Axle {
    /**
     * Renederer for 3D graphics
     *
     * All the functionality of this class is NOT THREAD SAFE and should only be accessed by the render thread.
     * */
    class Renderer {
    public:
        static void Init();
        static void Shutdown();

        static void BeginScene(Camera& camera);
        static void EndScene();

        static void Submit(const Ref<Shader>& shader,
                           const Ref<VertexArray>& vertexArray,
                           const glm::mat4& transform = glm::mat4(1.0f));

        static void OnFrameBufferResize(u32 widht, u32 height);

    private:
        struct SceneData {
            glm::mat4 ViewProjectionMatrix;
            glm::mat4 ViewMatrix;
            glm::mat4 ProjectionMatrix;
        };

        static SceneData* s_SceneData;
    };
} // namespace Axle
