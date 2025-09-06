#pragma once

#include "axpch.hpp"

#include "Other/Observer.hpp"
#include "Core.hpp"
#include "Window/Window.hpp"
#include "Events/Event.hpp"

#include <GLFW/glfw3.h>

namespace Axle {
    void ErrorCallback(int error, const char* description);

    class AXLE_API Application {
    public:
        Application();
        virtual ~Application();

        void Run();

    private:
        void OnWindowClose(Event* event);

        std::unique_ptr<Window> m_Window;
        bool m_Running = true;
    };

    // To be defined in client
    Application* CreateApplication();
} // namespace Axle
