#pragma once

#include "axpch.hpp"

#include "Layer/LayerStack.hpp"
#include "Core.hpp"
#include "Window/Window.hpp"
#include "Events/Event.hpp"

namespace Axle {
    class AXLE_API Application {
    public:
        Application();
        virtual ~Application();

        void Run();

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* layer);

        inline Window& GetWindow() {
            return *m_Window;
        }

        inline static Application& GetInstance() {
            return *s_Instance;
        }

    private:
        void OnWindowClose(Event* event);

        std::unique_ptr<Window> m_Window;
        bool m_Running = true;
        LayerStack* m_LayerStack;

        static Application* s_Instance;
    };

    // To be defined in client
    Application* CreateApplication();
} // namespace Axle
