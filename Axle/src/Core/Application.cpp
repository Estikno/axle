#include "axpch.hpp"

#include "Application.hpp"

#include "Logger/Log.hpp"
#include "Input/Input.hpp"
#include "Error/Panic.hpp"
#include "Core/Events/EventHandler.hpp"
#include "Core/Events/Event.hpp"
#include "Core/Layer/Layer.hpp"
#include "Window/Window.hpp"

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
    }

    Application::~Application() {
        // Destroy all remaining events
        EventHandler::GetInstance().DestroyEvents();

        AX_CORE_INFO("Stopping the engine...");

        // We need to delete the window before terminating glfw
        m_Window.reset();
    }

    void Application::PushLayer(Layer* layer) {
        m_LayerStack.PushLayer(layer);
        layer->OnAttach();
    }

    void Application::PushOverlay(Layer* layer) {
        m_LayerStack.PushOverlay(layer);
        layer->OnDetach();
    }

    void Application::Run() {
        while (m_Running) {
            EventHandler::GetInstance().ProcessEvents();
            Input::Update();

            for (Layer* layer : m_LayerStack)
                layer->OnUpdate();

            m_Window->OnUpdate();
        }
    }

    void Application::OnWindowClose(Event* event) {
        m_Running = false;
    }
} // namespace Axle
