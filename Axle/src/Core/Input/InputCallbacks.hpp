#pragma once

#include "axpch.hpp"

#include <GLFW/glfw3.h>

// Callback functions for input handling regarding GLFW
namespace Axle {
    void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
    void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
} // namespace Axle
