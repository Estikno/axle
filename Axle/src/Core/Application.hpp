#pragma once

#include "axpch.hpp"

#include "Core.hpp"

#include <GLFW/glfw3.h>

namespace Axle {
    void ErrorCallback(int error, const char* description);

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
