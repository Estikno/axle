#include "axpch.hpp"

#include "Core/Events/Event.hpp"
#include "Core/Events/EventHandler.hpp"
#include "Core/Input/InputState.hpp"
#include "Core/Logger/Log.hpp"
#include "ImGuiLayer.hpp"
#include "Core/Application.hpp"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#ifdef AX_DEBUG
#    include "Debug/Console.hpp"
#    include "Debug/Overlay.hpp"
#endif // AXLE_TESTING

namespace Axle {
    ImGuiLayer::ImGuiLayer()
        : Layer("ImGui") {}

    void ImGuiLayer::OnAttach() {}

    void ImGuiLayer::OnUpdate(f64 FixedDeltaTime) {}

    void ImGuiLayer::OnDettach() {
        AX_CORE_INFO("{0} layer detached", m_DebugName);
        Layer::OnDettach();
    }

    void ImGuiLayer::OnAttachRender() {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(
            Application::GetInstance().GetWindow().GetNativeWindow(),
            true); // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
        ImGui_ImplOpenGL3_Init();

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

#ifdef AX_DEBUG
        // Setup sonsole
        m_Console.Init();
        // Key to toggle the console
        EventHandler::GetInstance().Subscribe(
            [&](Event& event) {
                if (std::get<std::array<u16, 8>>(event.GetContext().raw_data).at(0) == static_cast<u16>(Keys::F1))
                    m_Console.Open = !m_Console.Open;
            },
            EventType::KeyPressed,
            EventCategory::Input);
        // Key to toggle the overlay
        EventHandler::GetInstance().Subscribe(
            [&](Event& event) {
                if (std::get<std::array<u16, 8>>(event.GetContext().raw_data).at(0) == static_cast<u16>(Keys::F2))
                    m_OpenOverlay = !m_OpenOverlay;
            },
            EventType::KeyPressed,
            EventCategory::Input);
#endif // AXLE_TESTING
    }

    void ImGuiLayer::OnRender(f64 DeltaTime) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Show demo window! :)
        // ImGui::ShowDemoWindow();

        // Console
#ifdef AX_DEBUG
        if (m_Console.Open)
            m_Console.Draw("Debug console", &m_Console.Open);
        // Debug Information
        if (m_OpenOverlay)
            ShowSimpleOverlay(&m_OpenOverlay, DeltaTime);
#endif // AXLE_TESTING

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void ImGuiLayer::OnDettachRender() {
#ifdef AX_DEBUG
        // Destroy console
        m_Console.Destroy();
#endif // AXLE_TESTING

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
} // namespace Axle
