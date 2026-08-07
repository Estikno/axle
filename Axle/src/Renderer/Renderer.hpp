#pragma once

#include "axpch.hpp"

#include "Renderer/Primitives/VertexArray.hpp"
#include "Renderer/Camera/Camera.hpp"
#include "Renderer/Shaders/Shader.hpp"
#include "Renderer/Primitives/FrameBuffer.hpp"
#include "Renderer/Skybox/Skybox.hpp"
#include "Renderer/Textures/Texture.hpp"
#include "Renderer/Primitives/UniformBuffer.hpp"

#include "glm/fwd.hpp"

namespace Axle {
    struct SceneData {
        // Camera
        glm::mat4 ViewMatrix;
        glm::mat4 ProjectionMatrix;
        glm::mat4 ViewProjectionMatrix;
        glm::vec3 CameraPosition;

        // Enviorment
        Ref<Skybox> SkyboxScene;
        // irradiance map, prefiltered env map, BRDF LUT for IBL, ...

        // Lighting

        // Frame info
        f64 Time;

        // Target
        Ref<FrameBuffer> RenderTarget;
    };

    struct SceneHandle {
        SceneData* Data;
        u32 StackIndex;

        SceneHandle(const SceneHandle&) = delete;
        SceneHandle& operator=(const SceneHandle&) = delete;

        SceneHandle(SceneHandle&&) = default;
        SceneHandle& operator=(SceneHandle&&) = default;

        SceneHandle(SceneData* data, u32 stackIndex)
            : Data(data),
              StackIndex(stackIndex) {}
    };

    /**
     * Renederer for 3D graphics
     *
     * All the functionality of this class is NOT THREAD SAFE and must only be accessed by the render thread.
     * */
    class Renderer {
    public:
        static void Init();
        static void Shutdown();

        static SceneHandle BeginScene(Camera& camera, const Ref<Skybox>& skybox, const Ref<FrameBuffer>& target);
        static void EndScene(SceneHandle& handle);

        static void Submit(const Ref<Shader>& shader,
                           const Ref<VertexArray>& vertexArray,
                           const glm::mat4& transform = glm::mat4(1.0f));
        static void Submit(const Ref<Texture2D>& texture);

        static void OnFrameBufferResize(u32 widht, u32 height);

    private:
        static void BindSceneState(SceneData& data);

        struct ScenePOD {
            // Camera
            glm::mat4 ViewMatrix;
            glm::mat4 ProjectionMatrix;
            glm::mat4 ViewProjectionMatrix;
            glm::vec3 CameraPosition;

            // Frame info
            // TODO: Find a better way of sending the time variable as this gets less presition with the more time it
            // passes. We may only send time in determined cases.
            f64 Time;

            ScenePOD(const SceneData& data)
                : ViewMatrix(data.ViewMatrix),
                  ProjectionMatrix(data.ProjectionMatrix),
                  ViewProjectionMatrix(data.ViewProjectionMatrix),
                  CameraPosition(data.CameraPosition) {}
        };

        static std::vector<SceneData> s_SceneData;

        // NOTE: Temporal variables
        static Ref<UniformBuffer> s_UBO;
        static Ref<VertexArray> s_DTextureVAO;
        static Ref<Shader> s_TexShader;
    };
} // namespace Axle
