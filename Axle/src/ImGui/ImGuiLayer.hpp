#pragma once

#include "axpch.hpp"

#include "Core/Events/Event.hpp"
#include "Core/Layer/Layer.hpp"
#include "Core/Types.hpp"

#include "Debug/Console.hpp"
#include "Debug/Inspector.hpp"
#include "Debug/FPS.hpp"

namespace Axle {
    class ImGuiLayer : public Layer {
    public:
        ImGuiLayer();
        ~ImGuiLayer() override = default;

        void OnAttach() override;
        void OnUpdate(f64 fixedDeltaTime) override;
        void OnDettach() override;

        void OnAttachRender() override;
        void OnRender(f64 deltaTime) override;
        void OnDettachRender() override;

        void OnEvent(Event& event) override;

    private:
        bool OnKeyPressed(KeyPressedEvent& event);
        bool OnKeyIsPressed(KeyIsPressedEvent& event);
        bool OnKeyReleased(KeyReleasedEvent& event);
        bool OnMouseButtonPressed(MouseButtonPressedEvent& event);
        bool OnMouseButtonIsPressed(MouseButtonIsPressedEvent& event);
        bool OnMouseButtonReleased(MouseButtonReleasedEvent& event);

        Debug::DebugConsole m_Console;
        bool m_OpenOverlay = true;
        bool m_OpenInspector = true;

        Debug::FPSCounter m_FPSCounter;
    };
} // namespace Axle
