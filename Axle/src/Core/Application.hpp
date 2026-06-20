#pragma once

#include "axpch.hpp"

#include "Layer/LayerStack.hpp"
#include "Core.hpp"
#include "Window/Window.hpp"
#include "Events/Event.hpp"

#include <CoroWeaver.hpp>

namespace Axle {
    class AXLE_API Application {
    public:
        Application();
        virtual ~Application();

        void Run();
        void Close();

        void OnEvent(Event& event);

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* layer);

        /**
         * Returns the window of the application.
         * Caution: This functions must only be called by the render thread.
         *
         * If you need to check for example if the window is resized, use events.
         *
         * @returns A reference to the window.
         * */
        inline Window& GetWindow() const {
            return *m_Window;
        }

        inline static Application& GetInstance() {
            return *s_Instance;
        }

    private:
        /**
         * Create the main window of the application. Initializes everything that is needed as well.
         *
         * This method should be called from the thread that will use GLFW and OpenGL
         * */
        cw::JobCoroutine<void> CreateMainWindow();

        cw::JobCoroutine<void> UpdateLoop();
        cw::JobCoroutine<void> RenderLoop();

        bool OnWindowClose(WindowCloseEvent& event);
        bool OnKeyPressed(KeyPressedEvent& event);

        static Application* s_Instance;

        std::unique_ptr<Window> m_Window;
        std::atomic<bool> m_Running = false;
        LayerStack* m_LayerStack;

        /// Time between update loops
        const f64 m_DeltaTime = 1.0 / 60.0;
    };

    // To be defined in client
    Application* CreateApplication();
} // namespace Axle
