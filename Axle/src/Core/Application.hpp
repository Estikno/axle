#pragma once

#include "axpch.hpp"

#include "Core.hpp"

#include <GLFW/glfw3.h>

namespace Axle {
    static void ErrorCallback(int error, const char* description);
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void CursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

    class AXLE_API Application {
      public:
        Application();
        virtual ~Application();

        void Run();

    protected:
        GLFWwindow* m_window = nullptr;
        bool m_HasErrorInit = false;
    };

    // To be defined in client
    Application *CreateApplication();
} // namespace Axle
