#include "axpch.hpp"

#include "Core/Events/Event.hpp"
#include "Core/Events/EventHandler.hpp"
#include "Core/Input/InputState.hpp"
#include "Core/Logger/Log.hpp"
#include "ImGuiLayer.hpp"
#include "Core/Application.hpp"
#include "Core/Job/JobSystem.hpp"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "Debug/Console.hpp"
#include "Debug/Overlay.hpp"

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
        // TODO: In the future wire the inputs directly with the event system to IMGUI
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

        // Setup sonsole
        m_Console.Init();
    }

    void ImGuiLayer::OnRender(f64 DeltaTime) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Show demo window! :)
        // ImGui::ShowDemoWindow();

        // Console
        if (m_Console.Open)
            m_Console.Draw("Debug console", &m_Console.Open);
        // Debug Information
        if (m_OpenOverlay)
            Debug::ShowSimpleOverlay(&m_OpenOverlay, DeltaTime, JobSystem::GetInstance().GetAvailableJobs());

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void ImGuiLayer::OnDettachRender() {
        // Destroy console
        m_Console.Destroy();

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void ImGuiLayer::OnEvent(Event& event) {
        if (event.GetEventCategory() == EventCategory::Input)
            InputEvents(event);
    }

    void ImGuiLayer::InputEvents(Event& event) {
        // TODO: In the future wire the inputs directly with the event system to IMGUI
        // ImGuiIO& io = ImGui::GetIO();

        // FIX: There might be a race condition between the boolean variables as this method is not called by the render
        // one
        if (event.GetEventType() == EventType::KeyPressed) {
            if (std::get<std::array<u16, 8>>(event.GetContext().raw_data).at(0) == static_cast<u16>(Keys::F2)) {
                m_OpenOverlay = !m_OpenOverlay;
                event.Handle();
            }
            if (std::get<std::array<u16, 8>>(event.GetContext().raw_data).at(0) == static_cast<u16>(Keys::F1)) {
                m_Console.Open = !m_Console.Open;
                event.Handle();
            }
        }
    }
} // namespace Axle
