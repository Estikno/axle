#include "axpch.hpp"

#include "Window.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Error/Panic.hpp"

#include "Callbacks/InputCallbacks.hpp"
#include "Callbacks/WindowCallbacks.hpp"

#include <GLFW/glfw3.h>

namespace Axle {
    static bool s_IsGlfwInitialized = false;
    static u8 active_windows = 0;

    static void ErrorCallback(int error, const char* description) {
        AX_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
    }

    Window* Window::Create(const WindowProps& props) {
        return new Window(props);
    }

    Window::Window(const WindowProps& props) {
        m_Data.Title = props.Title;
        m_Data.Width = props.Width;
        m_Data.Height = props.Height;

        AX_CORE_INFO("Creating window with title: {0} ({1}, {2})", props.Title, props.Width, props.Height);

        if (!s_IsGlfwInitialized) {
            i32 success = glfwInit();
            AX_ASSERT(success, "Could not initialize GLFW!");

            glfwSetErrorCallback(ErrorCallback);
            s_IsGlfwInitialized = true;
        }

        m_Window = glfwCreateWindow((int) props.Width, (int) props.Height, props.Title.c_str(), nullptr, nullptr);
        AX_ASSERT(m_Window, "Failed to create GLFW window!");

        ++active_windows;

        glfwMakeContextCurrent(m_Window);
        // Set a way to retrieve the window data from the GLFW window easily
        glfwSetWindowUserPointer(m_Window, &m_Data);
        // By default enable VSync
        SetVSync(true);

        // Window callbacks
        glfwSetWindowCloseCallback(m_Window, WindowCloseCallback);
        glfwSetWindowSizeCallback(m_Window, WindowSizeCallback);

        // Input callbacks
        glfwSetKeyCallback(m_Window, KeyCallback);
        glfwSetCursorPosCallback(m_Window, CursorPositionCallback);
        glfwSetMouseButtonCallback(m_Window, MouseButtonCallback);
        glfwSetScrollCallback(m_Window, ScrollCallback);
    }

    Window::~Window() {
        glfwDestroyWindow(m_Window);
        --active_windows;
        if (active_windows == 0) {
            glfwTerminate();
            s_IsGlfwInitialized = false;
            AX_CORE_TRACE("GLFW terminated successfully.");
        }
    }

    void Window::OnUpdate() {
        glfwPollEvents();
        glfwSwapBuffers(m_Window);
    }

    void Window::SetVSync(bool enabled) {
        if (enabled) {
            glfwSwapInterval(1);
        } else {
            glfwSwapInterval(0);
        }

        m_Data.VSync = enabled;
    }
} // namespace Axle
