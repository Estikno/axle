#include "axpch.hpp"

#include "Application.hpp"

#include "Logger/Log.hpp"
#include "Input/Input.hpp"
#include "Error/Panic.hpp"
#include "Core/Events/EventHandler.hpp"
#include "Core/Events/Event.hpp"
#include "Core/Layer/Layer.hpp"
#include "Window/Window.hpp"

#include "ImGui/ImGuiLayer.hpp"
#include <glad/gl.h>

namespace Axle {
    Application* Application::s_Instance = nullptr;

    Application::Application() {
        AX_CORE_INFO("Starting the engine...");

        if (s_Instance != nullptr) {
            AX_PANIC("Application already exists!");
        }

        EventHandler::GetInstance().Subscribe(
            [&](Event* e) { OnWindowClose(e); }, EventType::WindowClose, EventCategory::Window);

        s_Instance = this;
        m_Window = std::unique_ptr<Window>(Window::Create());

        // Layers setup
        m_LayerStack = new LayerStack();
        PushOverlay(new ImGuiLayer());
    }

    Application::~Application() {
        // Destroy all remaining events
        EventHandler::GetInstance().DestroyEvents();

        AX_CORE_INFO("Stopping the engine...");

        // By deleting the layer stack, we also delete all layers and detach them
        delete m_LayerStack;

        // We delete the main window and termninate GLFW
        m_Window.reset();
    }

    void Application::PushLayer(Layer* layer) {
        AX_CORE_INFO("{0} layer attached", layer->GetName());
        m_LayerStack->PushLayer(layer);
        layer->OnAttach();
    }

    void Application::PushOverlay(Layer* layer) {
        AX_CORE_INFO("{0} overlay attached", layer->GetName());
        m_LayerStack->PushOverlay(layer);
        layer->OnAttach();
    }

    void Application::Update() {
        while (m_Running) {
            EventHandler::GetInstance().ProcessEvents();
            Input::Update();

            // Temporary background color
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            for (Layer* layer : *m_LayerStack)
                layer->OnUpdate();

            m_Window->OnUpdate();
        }
    }

    void Application::OnWindowClose(Event* event) {
        m_Running = false;
    }
} // namespace Axle
