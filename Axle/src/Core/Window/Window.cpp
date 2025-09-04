#include "axpch.hpp"

#include "Window.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Error/Panic.hpp"

#include <GLFW/glfw3.h>

namespace Axle {
    static bool s_IsGlfwInitialized = false;

    Window* Window::Create(const WindowProps& props) {
        return new Window(props);
    }

    Window::Window(const WindowProps& props) {
        m_Data.Title = props.Title;
        m_Data.Width = props.Width;
        m_Data.Height = props.Height;

        AX_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);

        if (!s_IsGlfwInitialized) {
            int success = glfwInit();
            AX_ASSERT(success, "Could not initialize GLFW!");

            s_IsGlfwInitialized = true;
        }

        m_Window = glfwCreateWindow((int) props.Width, (int) props.Height, props.Title.c_str(), nullptr, nullptr);
        glfwMakeContextCurrent(m_Window);
        // Set a way to retrieve the window data from the GLFW window easily
        glfwSetWindowUserPointer(m_Window, &m_Data);
        SetVSync(true);
    }
} // namespace Axle
