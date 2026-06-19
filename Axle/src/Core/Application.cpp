#include "axpch.hpp"

#include "Application.hpp"

#include "Logger/Log.hpp"
#include "Input/InputManager.hpp"
#include "Error/Panic.hpp"
#include "Core/Core.hpp"
#include "Core/Events/EventHandler.hpp"
#include "Core/Events/Event.hpp"
#include "Core/Input/InputState.hpp"
#include "Core/Layer/Layer.hpp"
#include "Window/Window.hpp"

#include "ImGui/ImGuiLayer.hpp"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

namespace Axle {
    Application* Application::s_Instance = nullptr;

    Application::Application() {
        AX_CORE_INFO(LogChannel::Core, "Starting the engine...");

        if (s_Instance != nullptr) {
            AX_PANIC(LogChannel::Core, "Application already exists!");
        }

        s_Instance = this;

        // Layers setup
        m_LayerStack = new LayerStack();
        PushOverlay(new ImGuiLayer());
    }

    Application::~Application() {
        AX_CORE_INFO(LogChannel::Core, "Stopping the engine...");

        // By deleting the layer stack, we also delete all layers and detach them
        delete m_LayerStack;
    }

    void Application::PushLayer(Layer* layer) {
        AX_CORE_INFO(LogChannel::Core, "{0} layer attached", layer->GetName());
        AX_ENSURE(!m_Running, LogChannel::Core, "Cannot push layers after Run() has been called — not thread safe");
        m_LayerStack->PushLayer(layer);
    }

    void Application::PushOverlay(Layer* layer) {
        AX_CORE_INFO(LogChannel::Core, "{0} overlay attached", layer->GetName());
        AX_ENSURE(!m_Running, LogChannel::Core, "Cannot push overlays after Run() has been called — not thread safe");
        m_LayerStack->PushOverlay(layer);
    }

    void Application::Run() {
        m_Running = true;
        std::barrier initBarrier(2);

        std::thread RenderThread([this, &initBarrier]() {
            m_Window = std::unique_ptr<Window>(Window::Create()); // GLFW + GLAD init
            initBarrier.arrive_and_wait();                        // signal main thread that init is done
            Render();
        });

        initBarrier.arrive_and_wait();
        Update();

        RenderThread.join();
    }

    void Application::Update() {
        AX_CORE_INFO(
            LogChannel::Core, "Welcome from thread {0}", std::hash<std::thread::id>{}(std::this_thread::get_id()));

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

            // Use this varible to make sure that update loops at a fixed rate
            lag += elapsed;

            while (lag >= m_DeltaTime) {
                // Update logic
                // --------------------------
                EventHandler::GetInstance().ProcessEvents(m_LayerStack->rbegin(), m_LayerStack->rend());
                InputManager::GetInstance().Update();

                for (Layer* layer : *m_LayerStack)
                    layer->OnUpdate(m_DeltaTime);
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
        AX_CORE_INFO(
            LogChannel::Core, "Welcome from thread {0}", std::hash<std::thread::id>{}(std::this_thread::get_id()));

        // // Sets up imporant stuff
        // m_Window = std::unique_ptr<Window>(Window::Create());

        for (Layer* layer : *m_LayerStack)
            layer->OnAttachRender();

        f64 previous = glfwGetTime();

        while (m_Running) {
            // Calculate each frame time
            f64 current = glfwGetTime();
            f64 elapsed = current - previous;
            previous = current;

            // Render logic
            // --------------------------

            // Temporary background color
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            for (Layer* layer : *m_LayerStack)
                layer->OnRender(elapsed);

            m_Window->OnUpdate();
            // --------------------------
        }

        for (Layer* layer : *m_LayerStack)
            layer->OnDettachRender();

        // We delete the main window and termninate GLFW
        m_Window.reset();
    }

    void Application::Close() {
        m_Running = false;
    }

    bool Application::OnWindowClose(WindowCloseEvent& event) {
        m_Running = false;
        return true;
    }

    void Application::OnEvent(Event& event) {
        EventDispatcher dispatcher(event);

        dispatcher.Dispatch<WindowCloseEvent>(AX_BIND_EVENT_FN(OnWindowClose));
        dispatcher.Dispatch<KeyPressedEvent>(AX_BIND_EVENT_FN(OnKeyPressed));
    }

    bool Application::OnKeyPressed(KeyPressedEvent& event) {
        if (event.GetKey() != Keys::Escape)
            return false;

        Close();
        return true;
    }
} // namespace Axle
