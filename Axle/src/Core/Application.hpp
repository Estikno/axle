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
        void Update();
        void Render();

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
        friend class BaseLayer;

        void OnWindowClose(Event& event);

        static Application* s_Instance;

        std::unique_ptr<Window> m_Window;
        std::atomic<bool> m_Running = false;
        LayerStack* m_LayerStack;

        /// Time between update loops
        const f64 m_DeltaTime = 1.0 / 60.0;
    };

    // To be defined in client
    Application* CreateApplication();

    // This will be the deepest layer of the engine
    class AXLE_TEST_API BaseLayer : public Layer {
    public:
        BaseLayer()
            : Layer("BaseLayer") {}
        ~BaseLayer() override = default;

        void OnAttach() override {}
        void OnUpdate(f64 FixedDeltaTime) override {}
        void OnDettach() override {}

        void OnAttachRender() override {}
        void OnRender(f64 DeltaTime) override {}
        void OnDettachRender() override {}

        void OnEvent(Event& event) override {
            if (event.GetEventCategory() == EventCategory::Window && event.GetEventType() == EventType::WindowClose) {
                Application::GetInstance().OnWindowClose(event);
                event.Handle();
            }
        }

    private:
    };
} // namespace Axle
