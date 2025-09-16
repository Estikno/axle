#include "axpch.hpp"

#include "Core/Logger/Log.hpp"
#include "ImGuiLayer.hpp"
#include "Core/Application.hpp"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

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
    }

    void ImGuiLayer::OnRender(f64 DeltaTime) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Show demo window! :)
        ImGui::ShowDemoWindow();

        // Custom FPS displayer
        ImGui::Begin("FPS");
        ImGui::Text("fps: %.1f", 1.0 / DeltaTime);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void ImGuiLayer::OnDettachRender() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
} // namespace Axle
