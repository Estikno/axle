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
#include "Renderer/Skybox/Skybox.hpp"
#include "Renderer/Renderer.hpp"
#include "Other/CustomTypes/Ref.hpp"
#include "Renderer/Textures/Texture.hpp"

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
        shader = Shader::Create("Sandbox/src/Shaders/default.bin");

        // Model
        model = Model("assets/tests/backpack/backpack.obj");

        InputManager::SetCursorMode(CursorMode::CursorDisabled);

        // Skybox
        skybox = Ref<Skybox>::Create("assets/tests/skybox1.png", "Sandbox/src/Shaders/skybox.bin");
    }

    void OnDettachRender() override {
        shader.Reset();
        model = Model();
        skybox.Reset();
    }

    void OnRender(f64 deltaTime) override {
        Camera& cam = Application::GetInstance().GetCamera();
        if (updateCamera.load())
            cam.GetPositioner()->Update(deltaTime);

        SceneHandle handle1 = Renderer::BeginScene(cam, nullptr, nullptr);

        Ref<Texture2D> tex = Ref<Texture2D>::Create(width, height, TextureFormat::RGB8, 0);
        Ref<FrameBuffer> fBuffer = Ref<FrameBuffer>::Create(tex, true, false);

        SceneHandle handle2 = Renderer::BeginScene(cam, skybox, fBuffer);

        glm::mat4 modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(
            modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        modelMatrix =
            glm::scale(modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f)); // it's a bit too big for our scene, so scale it down

        model.Draw(shader, modelMatrix);

        Renderer::EndScene(handle2);

        tex->Bind(0);
        Renderer::Submit(tex);

        Renderer::EndScene(handle1);
    }

    bool OnFrameBufferResize(FrameBufferResizeEvent& event) {
        width = (f32) event.GetWidth();
        height = (f32) event.GetHeight();
        return false;
    }

    bool OnKeyPressedEvent(KeyPressedEvent& event) {
        if (event.GetKey() == Keys::F4) {
            bool previous = updateCamera.load();
            updateCamera.store(!previous);

            InputManager::SetCursorMode((!previous ? CursorMode::CursorDisabled : CursorMode::CursorNormal));
        }

        return false;
    }

    void OnEvent(Event& event) override {
        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<FrameBufferResizeEvent>(AX_BIND_EVENT_FN(OnFrameBufferResize));
        dispatcher.Dispatch<KeyPressedEvent>(AX_BIND_EVENT_FN(OnKeyPressedEvent));
    }

private:
    Model model;
    Ref<Skybox> skybox;
    Ref<Shader> shader;
    std::atomic_bool updateCamera = true;

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
