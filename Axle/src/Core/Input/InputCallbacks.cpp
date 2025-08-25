#include "axpch.hpp"
#include "../Types.hpp"

#include "Input.hpp"
#include "../Logger/Log.hpp"
#include "InputState.hpp"

#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

namespace Axle {
    void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        // Convert the GLFW key to our own key enum
        Keys key_enum = ConvertGLFWKeys(key);

        if (key_enum == Keys::Unknown) {
            AX_CORE_WARN("Unknown key pressed: {0}", key);
            return;
        }

        if (action == GLFW_KEY_UNKNOWN) {
            AX_CORE_WARN("Unknown action for key: {0}", key);
            return;
        }

        // Sends the status to the input system
        Input::SetKey(key_enum, !(action == GLFW_RELEASE));
    }

    void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
        glm::vec2 position(static_cast<f32>(xpos), static_cast<f32>(ypos));
        Input::SetMousePosition(position);
    }

    void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        // Convert the GLFW button to our own mouse buttons enum
        MouseButtons button_enum = ConvertGLFWMouseButtons(button);

        if (button_enum == MouseButtons::Unknown) {
            AX_CORE_WARN("Unknown mouse button pressed: {0}", button);
            return;
        }

        Input::SetMouseButton(button_enum, action == GLFW_PRESS);
    }

    void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
        Input::SetMouseWheel(static_cast<f32>(yoffset));
    }
} // namespace Axle
