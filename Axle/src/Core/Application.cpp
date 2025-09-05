#include "axpch.hpp"

#include "Application.hpp"
#include "axpch.hpp"

#include "Logger/Log.hpp"
#include "Input/Input.hpp"
#include "Input/InputCallbacks.hpp"
#include "Error/Panic.hpp"
#include "Core/Events/EventHandler.hpp"
#include "Window/Window.hpp"

#include <GLFW/glfw3.h>

namespace Axle {
    void ErrorCallback(int error, const char* description) {
        AX_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
    }

    Application::Application() {
        AX_CORE_TRACE("Starting the engine...");

        if (!glfwInit()) {
            AX_PANIC("Failed to initialize GLFW");
        }

        glfwSetErrorCallback(ErrorCallback);

        m_Window = std::unique_ptr<Window>(Window::Create());

        // glfwSetKeyCallback(m_Window, KeyCallback);
        // glfwSetCursorPosCallback(m_Window, CursorPositionCallback);
        // glfwSetMouseButtonCallback(m_Window, MouseButtonCallback);
        // glfwSetScrollCallback(m_Window, ScrollCallback);
    }

    Application::~Application() {
        // Destroy all remaining events
        EventHandler::GetInstance().DestroyEvents();

        AX_CORE_TRACE("Stopping the engine...");

        // glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    void Application::Run() {
        while (m_Running) {
            m_Window->OnUpdate();

            // NOTE: Input state updating should be performed at the end of each frame.
            // The input is recorded in between the frame but the update happens at the end.
            Input::Update();
        }
    }
} // namespace Axle
