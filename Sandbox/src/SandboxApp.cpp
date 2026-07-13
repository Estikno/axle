#include <AxleApp.hpp>

#include "Core/Application.hpp"
#include "Core/Core.hpp"
#include "Core/Events/Event.hpp"
#include "Core/Input/InputManager.hpp"
#include "Core/Input/InputState.hpp"
#include "Core/Logger/Log.hpp"
#include "Renderer/Shaders/ShaderProgram.hpp"
#include "Renderer/Shaders/Shader.hpp"
#include "Renderer/Camera/Camera.hpp"
#include "Renderer/Meshes/Model.hpp"

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
        Shader vertexShader("Sandbox/src/Shaders/default.bin", ShaderType::Vertex);
        Shader fragmentShader("Sandbox/src/Shaders/default.bin", ShaderType::Fragment);
        program = ShaderProgram(vertexShader, fragmentShader);
        program.Use();

        // Model
        model = Model("assets/tests/backpack/backpack.obj");

        Application::GetInstance().GetCamera().ChangePositioner(&positioner);
        InputManager::SetCursorMode(CursorMode::CursorDisabled);
    }

    void OnDettachRender() override {}

    void OnRender(f64 deltaTime) override {
        positioner.Update(deltaTime);

        program.Use();
        glm::mat4 projection = glm::perspective(glm::radians(positioner.GetFOV()), width / height, 0.1f, 1000.0f);
        glm::mat4 view = positioner.GetViewMatrix();
        program.SetMat4("projection", projection);
        program.SetMat4("view", view);

        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(
            modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        modelMatrix =
            glm::scale(modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f)); // it's a bit too big for our scene, so scale it down
        program.SetMat4("model", modelMatrix);
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
    ShaderProgram program;
    CameraPositionerDebug positioner;
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
