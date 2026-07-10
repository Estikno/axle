#include <AxleApp.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Renderer/Shaders/ShaderProgram.hpp"
#include "Renderer/Shaders/Shader.hpp"
#include "Core/Application.hpp"
#include "Core/Core.hpp"
#include "Core/Error/Panic.hpp"
#include "Core/Events/Event.hpp"
#include "Core/Input/InputState.hpp"
#include "Renderer/Camera/Camera.hpp"
#include "Debug/Inspector.hpp"

using namespace Axle;

struct PerFrameData {
    glm::mat4 mvp;
};

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

    void OnAttachRender() override {}

    void OnDettachRender() override {}

    void OnRender(f64 deltaTime) override {}

    bool OnFrameBufferResize(FrameBufferResizeEvent& event) {
        width = (f32) event.GetWidth();
        height = (f32) event.GetHeight();
        return false;
    }

    bool OnMouseScroll(MouseScrollEvent& event) {
        positionerDebug.ProcessMouseScroll(event.GetYOffset());
        return false;
    }

    bool OnKeyPressed(KeyPressedEvent& event) {
        if (event.GetKey() == Keys::F4)
            updateCameraTrigger.store(true);

        return false;
    }

    void OnEvent(Event& event) override {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<FrameBufferResizeEvent>(AX_BIND_EVENT_FN(OnFrameBufferResize));
        dispatcher.Dispatch<MouseScrollEvent>(AX_BIND_EVENT_FN(OnMouseScroll));
        dispatcher.Dispatch<KeyPressedEvent>(AX_BIND_EVENT_FN(OnKeyPressed));
    }

    void UpdateCameraDebug() {}

private:
    ShaderProgram program;
    std::vector<u32> indices;

    Camera camera;
    CameraPositionerDebug positionerDebug;
    bool updateDebugCamera = false;
    std::atomic_bool updateCameraTrigger{false};

    GLuint dataIndices, dataVertices, VAO, texture, perFrameBuffer;

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
