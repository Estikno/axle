#include <AxleApp.hpp>

#include "Core/Application.hpp"
#include "Core/Core.hpp"
#include "Core/Events/Event.hpp"
#include "Core/Input/InputManager.hpp"
#include "Core/Input/InputState.hpp"
#include "Core/Logger/Log.hpp"
#include "Renderer/Shaders/Shader.hpp"
#include "Renderer/Camera/Camera.hpp"
#include "Renderer/Meshes/Model.hpp"
#include "Renderer/Shaders/ShaderManager.hpp"

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/trigonometric.hpp"

using namespace Axle;

class LearnLayer : public Axle::Layer {
public:
    LearnLayer()
        : Layer("Learn") {}
    ~LearnLayer() override = default;

    void OnAttach() override {}
    void OnUpdate(f64 fixedDeltaTime) override {}
    void OnDettach() override {
        AX_INFO("Learn layer detached");
        Layer::OnDettach();
    }

    void OnAttachRender() override {
        // Shaders
        u32 vertexShader = ShaderManager::CreateShader("Sandbox/src/Shaders/default.bin", ShaderType::Vertex);
        u32 fragmentShader = ShaderManager::CreateShader("Sandbox/src/Shaders/default.bin", ShaderType::Fragment);
        program = ShaderManager::CreateShaderProgram(vertexShader, fragmentShader);
        ShaderManager::UseProgram(program);

        // Model
        model = Model("assets/tests/backpack/backpack.obj");

        InputManager::SetCursorMode(CursorMode::CursorDisabled);
    }

    void OnDettachRender() override {
        model = Model();
    }

    void OnRender(f64 deltaTime) override {
        Camera& cam = Application::GetInstance().GetCamera();
        cam.GetPositioner()->Update(deltaTime);

        ShaderManager::UseProgram(program);
        glm::mat4 projection =
            glm::perspective(glm::radians(cam.GetPositioner()->GetFOV()), width / height, 0.1f, 1000.0f);
        glm::mat4 view = cam.GetPositioner()->GetViewMatrix();
        ShaderManager::SetMat4(program, "projection", projection);
        ShaderManager::SetMat4(program, "view", view);

        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(
            modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        modelMatrix =
            glm::scale(modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f)); // it's a bit too big for our scene, so scale it down
        ShaderManager::SetMat4(program, "model", modelMatrix);
        model.Draw(program);
    }

    bool OnFrameBufferResize(FrameBufferResizeEvent& event) {
        width = (f32) event.GetWidth();
        height = (f32) event.GetHeight();
        return false;
    }

    void OnEvent(Event& event) override {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<FrameBufferResizeEvent>(AX_BIND_EVENT_FN(OnFrameBufferResize));
    }

private:
    u32 program;
    Model model;

    f32 width = 1280.0f, height = 720.0f;
};

class Sandbox : public Axle::Application {
public:
    Sandbox() {
        PushLayer(new LearnLayer());
    }
    ~Sandbox() {}
};

Axle::Application* Axle::CreateApplication() {
    return new Sandbox();
}
