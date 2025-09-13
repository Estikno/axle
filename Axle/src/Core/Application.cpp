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
#include <GLFW/glfw3.h>

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
        for (Layer* layer : *m_LayerStack)
            layer->OnAttach();

        f64 previous = glfwGetTime();
        f64 lag = 0.0;

        while (m_Running) {
            // Calculate each frame time
            f64 current = glfwGetTime();
            f64 elapsed = current - previous;
            // Cap at 250 ms to avoid spiral of death
            if (elapsed > 0.25)
                elapsed = 0.25;
            previous = current;

            // TODO: We could interpolate rendering

            // Use this varible to make sure that update loops at a fixed rate
            lag += elapsed;

            while (lag >= m_DeltaTime) {
                // Update logic
                // --------------------------
                EventHandler::GetInstance().ProcessEvents();
                Input::Update();

                for (Layer* layer : *m_LayerStack)
                    layer->OnUpdate();
                // --------------------------
                lag -= m_DeltaTime;
            }

            // Sleep a bit to avoid pegging a CPU core
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        for (Layer* layer : *m_LayerStack)
            layer->OnDettach();
    }

    void Application::Render() {
        for (Layer* layer : *m_LayerStack)
            layer->OnAttachRender();

        while (m_Running) {
            // Temporary background color
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            for (Layer* layer : *m_LayerStack)
                layer->OnRender();

            m_Window->OnUpdate();
        }

        for (Layer* layer : *m_LayerStack)
            layer->OnDettachRender();
    }

    void Application::OnWindowClose(Event* event) {
        m_Running = false;
    }
} // namespace Axle
