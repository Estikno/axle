#include "axpch.hpp"

#include "Core/Core.hpp"
#include "Core/Events/Event.hpp"
#include "Core/Input/InputState.hpp"
#include "Core/Logger/Log.hpp"
#include "ImGuiLayer.hpp"
#include "Core/Application.hpp"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "Debug/Console.hpp"
#include "Debug/Overlay.hpp"
#include "Debug/Inspector.hpp"

namespace Axle {
    ImGuiLayer::ImGuiLayer()
        : Layer("ImGui"),
          m_FPSCounter(0.5f) {
        Debug::Inspector::Init();
    }

    void ImGuiLayer::OnAttach() {}

    void ImGuiLayer::OnUpdate(f64 fixedDeltaTime) {}

    void ImGuiLayer::OnDettach() {
        AX_CORE_INFO(LogChannel::Debug, "{0} layer detached", m_DebugName);
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

    void ImGuiLayer::OnRender(f64 deltaTime) {
        m_FPSCounter.Tick(static_cast<f32>(deltaTime));

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Inspector
        if (m_OpenInspector)
            Debug::Inspector::Draw("Inspector", &m_OpenInspector);

        // Console
        if (m_Console.Open)
            m_Console.Draw("Debug console", &m_Console.Open);
        // Debug Information
        if (m_OpenOverlay)
            Debug::ShowSimpleOverlay(&m_OpenOverlay, deltaTime, m_FPSCounter);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void ImGuiLayer::OnDettachRender() {
        // Destroy console
        m_Console.Destroy();

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        Debug::Inspector::Shutdown();
    }

    void ImGuiLayer::OnEvent(Event& event) {
        EventDispatcher dispatcher(event);

        dispatcher.Dispatch<KeyPressedEvent>(AX_BIND_EVENT_FN(OnKeyPressed));
        dispatcher.Dispatch<KeyIsPressedEvent>(AX_BIND_EVENT_FN(OnKeyIsPressed));
        dispatcher.Dispatch<KeyReleasedEvent>(AX_BIND_EVENT_FN(OnKeyReleased));
        dispatcher.Dispatch<MouseButtonPressedEvent>(AX_BIND_EVENT_FN(OnMouseButtonPressed));
        dispatcher.Dispatch<MouseButtonIsPressedEvent>(AX_BIND_EVENT_FN(OnMouseButtonIsPressed));
        dispatcher.Dispatch<MouseButtonReleasedEvent>(AX_BIND_EVENT_FN(OnMouseButtonReleased));
    }

    bool ImGuiLayer::OnKeyPressed(KeyPressedEvent& event) {
        // FIX: There might be a race condition between the boolean variables as this method is not called by the render
        // one
        if (event.GetKey() == Keys::F1)
            m_Console.Open = !m_Console.Open;

        if (event.GetKey() == Keys::F2)
            m_OpenOverlay = !m_OpenOverlay;

        if (event.GetKey() == Keys::F3)
            m_OpenInspector = !m_OpenInspector;

        ImGuiIO& io = ImGui::GetIO();
        return io.WantCaptureKeyboard;
    }

    bool ImGuiLayer::OnKeyIsPressed(KeyIsPressedEvent& event) {
        ImGuiIO& io = ImGui::GetIO();
        return io.WantCaptureKeyboard;
    }

    bool ImGuiLayer::OnKeyReleased(KeyReleasedEvent& event) {
        ImGuiIO& io = ImGui::GetIO();
        return io.WantCaptureKeyboard;
    }
    bool ImGuiLayer::OnMouseButtonPressed(MouseButtonPressedEvent& event) {
        ImGuiIO& io = ImGui::GetIO();
        return io.WantCaptureMouse;
    }
    bool ImGuiLayer::OnMouseButtonIsPressed(MouseButtonIsPressedEvent& event) {
        ImGuiIO& io = ImGui::GetIO();
        return io.WantCaptureMouse;
    }
    bool ImGuiLayer::OnMouseButtonReleased(MouseButtonReleasedEvent& event) {
        ImGuiIO& io = ImGui::GetIO();
        return io.WantCaptureMouse;
    }
} // namespace Axle
