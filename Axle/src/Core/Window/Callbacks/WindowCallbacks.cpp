#include "axpch.hpp"

#include "Core/Types.hpp"

#include "Core/Logger/Log.hpp"
#include "WindowCallbacks.hpp"
#include "Core/Events/EventHandler.hpp"
#include "Core/Events/Event.hpp"
#include "../Window.hpp"
#include "Renderer/Renderer.hpp"

#include <GLFW/glfw3.h>

namespace Axle {
    void WindowCloseCallback(GLFWwindow* window) {
        WindowCloseEvent event;
        AX_SUBMIT_EVENT(std::move(event));
    }

    void WindowSizeCallback(GLFWwindow* window, int width, int height) {
        // Update the window data
        WindowData* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
        data->Width = static_cast<u32>(width);
        data->Height = static_cast<u32>(height);

        // Log the new window size
        AX_CORE_TRACE(LogChannel::Window, "Window resized to: {0}x{1}", width, height);

        // Send the event at the end to notify
        WindowResizeEvent event(static_cast<u32>(width), static_cast<u32>(height));
        AX_SUBMIT_EVENT(std::move(event));
    }

    void FrameBufferSizeCallback(GLFWwindow* window, int width, int height) {
        // Update the window data
        WindowData* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
        data->FramebufferWidth = static_cast<u32>(width);
        data->FramebufferHeight = static_cast<u32>(height);

        // Notify the renderer of the change (We do it here because this method is guaranteed to be excecuted by the
        // rendere thread)
        Renderer::OnFrameBufferResize(static_cast<u32>(width), static_cast<u32>(height));

        AX_SUBMIT_EVENT(FrameBufferResizeEvent(static_cast<u32>(width), static_cast<u32>(height)));
    }
} // namespace Axle
