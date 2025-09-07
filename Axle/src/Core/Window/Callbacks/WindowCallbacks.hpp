#pragma once

#include "axpch.hpp"

#include <GLFW/glfw3.h>

// Callback functions for window handling regarding GLFW
namespace Axle {
    void WindowCloseCallback(GLFWwindow* window);
    void WindowSizeCallback(GLFWwindow* window, int width, int height);
} // namespace Axle
