#include "axpch.hpp"

#include "Core/Types.hpp"

#include "Core/Logger/Log.hpp"
#include "WindowCallbacks.hpp"
#include "Core/Events/EventHandler.hpp"
#include "Core/Events/Event.hpp"
#include "../Window.hpp"

#include <GLFW/glfw3.h>

namespace Axle {
    void WindowCloseCallback(GLFWwindow* window) {
        Event* event = new Event(EventType::WindowClose, EventCategory::Window);
        AX_ADD_EVENT(event);
    }

    void WindowSizeCallback(GLFWwindow* window, int width, int height) {
        if (width == 0 || height == 0) {
            AX_CORE_WARN("Window minimized or has zero size!");
            return;
        }

        // Update the window data
        WindowData* data = static_cast<WindowData*>(glfwGetWindowUserPointer(window));
        data->Width = static_cast<u32>(width);
        data->Height = static_cast<u32>(height);

        // Log the new window size
        AX_CORE_TRACE("Window resized to: {0}x{1}", width, height);

        // Send the event at the end to ensure the size is correctly updated
        Event* event = new Event(EventType::WindowResize, EventCategory::Window);
        event->GetContext().u32_values[0] = static_cast<u32>(width);
        event->GetContext().u32_values[1] = static_cast<u32>(height);
        AX_ADD_EVENT(event);
    }
} // namespace Axle
