#include "axpch.hpp"

#include "Application.hpp"

#include "Logger/Log.hpp"
#include "Input/InputManager.hpp"
#include "Error/Panic.hpp"
#include "Core/Core.hpp"
#include "Core/Events/EventHandler.hpp"
#include "Core/Events/Event.hpp"
#include "Core/Input/InputState.hpp"
#include "Core/Layer/Layer.hpp"
#include "Window/Window.hpp"
#include "Types.hpp"
#include "Renderer/Textures/TextureManager.hpp"
#include "Renderer/Shaders/ShaderManager.hpp"
#include "Renderer/Camera/Camera.hpp"

#include "ImGui/ImGuiLayer.hpp"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <CoroWeaver.hpp>

namespace Axle {
    static std::stop_source source;
    static std::atomic<bool> renderDone{false};
    static cw::ThreadAffinity mainId;

    Application* Application::s_Instance = nullptr;

    struct Application::AppInternalManagement {
        /**
         * Create the main window of the application. Initializes everything that is needed as well.
         *
         * This method should be called from the thread that will use GLFW and OpenGL
         * */
        static cw::JobCoroutine<void> CreateMainWindow(Application* app);

        static cw::JobCoroutine<void> UpdateLoop(Application* app);
        static cw::JobCoroutine<void> RenderLoop(Application* app);
    };

    Application::Application()
        : m_Camera(new CameraPositionerDebug(), true) {
        AX_CORE_INFO(LogChannel::Core, "Starting the engine...");

        if (s_Instance != nullptr) {
            AX_PANIC(LogChannel::Core, "Application already exists!");
        }

        s_Instance = this;

        // Layers setup
        m_LayerStack = new LayerStack();
        PushOverlay(new ImGuiLayer());
    }

    Application::~Application() {
        AX_CORE_INFO(LogChannel::Core, "Stopping the engine...");

        // By deleting the layer stack, we also delete all layers and detach them
        delete m_LayerStack;
    }

    void Application::PushLayer(Layer* layer) {
        AX_CORE_INFO(LogChannel::Core, "{0} layer attached", layer->GetName());
        AX_ENSURE(!m_Running, LogChannel::Core, "Cannot push layers after Run() has been called — not thread safe");
        m_LayerStack->PushLayer(layer);
    }

    void Application::PushOverlay(Layer* layer) {
        AX_CORE_INFO(LogChannel::Core, "{0} overlay attached", layer->GetName());
        AX_ENSURE(!m_Running, LogChannel::Core, "Cannot push overlays after Run() has been called — not thread safe");
        m_LayerStack->PushOverlay(layer);
    }

    void Application::Run() {
        m_Running.store(true, std::memory_order_release);

        mainId = cw::JobSystem::GetInstance().ConvertToWorkerThread();

        // Schedule the main loop to the main thread's id
        cw::JobCoroutine<void> updateCor = AppInternalManagement::UpdateLoop(this);
        cw::JobSystem::GetInstance().Schedule(updateCor, mainId);

        // The main thread will now be a worker
        cw::JobSystem::GetInstance().RunWorkerUntil(source.get_token());

        cw::JobSystem::GetInstance().DeregisterWorkerThread();

        // We shut down the system here so everything gets deleted correctly (GLFW, OpenGL, ...)
        TextureManager::Shutdown();
        ShaderManager::Shutdown();
        cw::JobSystem::Shutdown();
    }


    cw::JobCoroutine<void> Application::AppInternalManagement::CreateMainWindow(Application* app) {
        app->m_Window = std::unique_ptr<Window>(Window::Create()); // GLFW + GLAD init
        co_return;
    }

    cw::JobCoroutine<void> Application::AppInternalManagement::UpdateLoop(Application* app) {
        // Create the main window on the render thread and wait so we are sure to have glfw initiated
        co_await cw::WhenAll(RENDER_THREAD_ID, AppInternalManagement::CreateMainWindow(app));

        // Schedule the render loop
        cw::JobCoroutine<void> renderCor = AppInternalManagement::RenderLoop(app);
        cw::JobSystem::GetInstance().Schedule(renderCor, RENDER_THREAD_ID);

        // Main loop logic
        // ---------------
        for (Layer* layer : *(app->m_LayerStack))
            cw::JobSystem::GetInstance().Schedule(
                [layer, app]() { layer->OnAttach(); }, cw::JobPriority::Medium, cw::InvalidThreadIndex, LAYERS_TAG);

        AX_SCHEDULE_TAG_AND_WAIT(LAYERS_TAG);

        f64 previous = glfwGetTime();
        f64 lag = 0.0;

        while (app->m_Running.load(std::memory_order_acquire)) {
            // Calculate each frame time
            f64 current = glfwGetTime();
            f64 elapsed = current - previous;
            // Cap at 250 ms to avoid spiral of death
            if (elapsed > 0.25)
                elapsed = 0.25;
            previous = current;

            // Use this varible to make sure that update loops at a fixed rate
            lag += elapsed;

            while (lag >= app->m_DeltaTime) {
                // Update logic
                // --------------------------
                cw::JobSystem::GetInstance().Schedule(
                    [app]() { EventHandler::ProcessEvents(app->m_LayerStack->rbegin(), app->m_LayerStack->rend()); },
                    cw::JobPriority::Medium,
                    cw::InvalidThreadIndex,
                    EVENT_INPUT_TAG);

                cw::JobSystem::GetInstance().Schedule(
                    []() { InputManager::Update(); }, cw::JobPriority::Medium, cw::InvalidThreadIndex, EVENT_INPUT_TAG);

                AX_SCHEDULE_TAG_AND_WAIT(EVENT_INPUT_TAG);

                for (Layer* layer : *(app->m_LayerStack))
                    cw::JobSystem::GetInstance().Schedule([layer, app]() { layer->OnUpdate(app->m_DeltaTime); },
                                                          cw::JobPriority::Medium,
                                                          cw::InvalidThreadIndex,
                                                          LAYERS_TAG);

                AX_SCHEDULE_TAG_AND_WAIT(LAYERS_TAG);
                // --------------------------
                lag -= app->m_DeltaTime;
            }

            const f64 alpha = lag / app->m_DeltaTime;

            for (Layer* layer : *(app->m_LayerStack))
                cw::JobSystem::GetInstance().Schedule([layer, alpha]() { layer->CommitSnapshot(alpha); },
                                                      cw::JobPriority::Medium,
                                                      cw::InvalidThreadIndex,
                                                      LAYERS_TAG);

            AX_SCHEDULE_TAG_AND_WAIT(LAYERS_TAG);

            // Sleep until the next tick is due, minus a small margin
            f64 timeUntilNextTick = app->m_DeltaTime - lag;
            if (timeUntilNextTick > 0.002) // only sleep if more than 2ms away
                std::this_thread::sleep_for(std::chrono::duration<f64>(timeUntilNextTick - 0.001)); // wake 1ms early
        }

        for (Layer* layer : *(app->m_LayerStack))
            cw::JobSystem::GetInstance().Schedule(
                [layer]() { layer->OnDettach(); }, cw::JobPriority::Medium, cw::InvalidThreadIndex, LAYERS_TAG);

        AX_SCHEDULE_TAG_AND_WAIT(LAYERS_TAG);

        source.request_stop();
        co_return;
    }

    cw::JobCoroutine<void> Application::AppInternalManagement::RenderLoop(Application* app) {
        // Sets up imporant stuff (already done in CreateMainWindow)
        // m_Window = std::unique_ptr<Window>(Window::Create());

        for (Layer* layer : *(app->m_LayerStack))
            layer->OnAttachRender();

        f64 previous = glfwGetTime();

        while (app->m_Running.load(std::memory_order_acquire)) {
            // Calculate each frame time
            f64 current = glfwGetTime();
            f64 elapsed = current - previous;
            previous = current;

            // Render logic
            // --------------------------

            // Temporary background color
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            for (Layer* layer : *(app->m_LayerStack))
                layer->OnRender(elapsed);

            app->m_Window->OnUpdate();
            // --------------------------
        }

        for (Layer* layer : *(app->m_LayerStack))
            layer->OnDettachRender();

        // We delete the main window and termninate GLFW
        app->m_Window.reset();

        renderDone.store(true, std::memory_order_release);
        renderDone.notify_all();

        co_return;
    }

    void Application::Close() {
        m_Running.store(false, std::memory_order_release);
    }

    bool Application::OnWindowClose(WindowCloseEvent& event) {
        m_Running.store(false, std::memory_order_release);
        return true;
    }

    void Application::OnEvent(Event& event) {
        EventDispatcher dispatcher(event);

        dispatcher.Dispatch<WindowCloseEvent>(AX_BIND_EVENT_FN(OnWindowClose));
        dispatcher.Dispatch<KeyPressedEvent>(AX_BIND_EVENT_FN(OnKeyPressed));
    }

    bool Application::OnKeyPressed(KeyPressedEvent& event) {
        if (event.GetKey() != Keys::Escape)
            return false;

        Close();
        return true;
    }
} // namespace Axle
