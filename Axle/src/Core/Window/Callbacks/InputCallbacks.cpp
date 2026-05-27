#include "axpch.hpp"

#include "Core/Types.hpp"

#include "Core/Input/InputManager.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Input/InputState.hpp"
#include "InputCallbacks.hpp"
#include "Core/Events/EventHandler.hpp"
#include "Core/Events/Event.hpp"

#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

namespace Axle {
    void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        // NOTE: Temporal delete window when escape is pressed
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            Event event(EventType::WindowClose, EventCategory::Window);
            AX_DISPATCH_EVENT(std::move(event));
        }

        // The conversion is direct since I use the same key codes
        Keys key_enum = static_cast<Keys>(key);

        if (key == GLFW_KEY_UNKNOWN) {
            AX_CORE_WARN("Unknown key pressed: {0}", key);
            return;
        }

        if (action == GLFW_KEY_UNKNOWN) {
            AX_CORE_WARN("Unknown action for key: {0}", key);
            return;
        }

        // Sends the status to the input system
        InputManager::GetInstance().SetKey(key_enum, !(action == GLFW_RELEASE));
    }

    void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
        glm::vec2 position(static_cast<f32>(xpos), static_cast<f32>(ypos));
        InputManager::GetInstance().SetMousePosition(position);
    }

    void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        // The conversion is direct since I use the same mouse codes
        MouseButtons button_enum = static_cast<MouseButtons>(button);

        InputManager::GetInstance().SetMouseButton(button_enum, action == GLFW_PRESS);
    }

    void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
        InputManager::GetInstance().SetMouseWheel(static_cast<f32>(yoffset));
    }
} // namespace Axle
