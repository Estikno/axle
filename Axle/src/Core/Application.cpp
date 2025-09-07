#include "axpch.hpp"

#include "Application.hpp"

#include "Logger/Log.hpp"
#include "Input/Input.hpp"
#include "Error/Panic.hpp"
#include "Core/Events/EventHandler.hpp"
#include "Core/Events/Event.hpp"
#include "Window/Window.hpp"

namespace Axle {
    Application::Application() {
        AX_CORE_INFO("Starting the engine...");

        EventHandler::GetInstance().Subscribe(
            [&](Event* e) { OnWindowClose(e); }, EventType::WindowClose, EventCategory::Window);

        m_Window = std::unique_ptr<Window>(Window::Create());
    }

    Application::~Application() {
        // Destroy all remaining events
        EventHandler::GetInstance().DestroyEvents();

        AX_CORE_INFO("Stopping the engine...");

        // We need to delete the window before terminating glfw
        m_Window.reset();
    }

    void Application::Run() {
        while (m_Running) {
            m_Window->OnUpdate();
            EventHandler::GetInstance().ProcessEvents();

            // NOTE: Input state updating should be performed at the end of each frame.
            // The input is recorded in between the frame but the update happens at the end.
            Input::Update();
        }
    }

    void Application::OnWindowClose(Event* event) {
        m_Running = false;
    }
} // namespace Axle
