#pragma once

#include "Core/Events/Event.hpp"
#include "Core/Layer/Layer.hpp"
#include "Core/Types.hpp"

#include "Debug/Console.hpp"

namespace Axle {
    class ImGuiLayer : public Layer {
    public:
        ImGuiLayer();
        ~ImGuiLayer() override = default;

        void OnAttach() override;
        void OnUpdate(f64 FixedDeltaTime) override;
        void OnDettach() override;

        void OnAttachRender() override;
        void OnRender(f64 DeltaTime) override;
        void OnDettachRender() override;

        void OnEvent(Event& event) override;

    private:
        bool OnKeyPressed(KeyPressedEvent& event);

        // Renderer variables
        Debug::DebugConsole m_Console;
        bool m_OpenOverlay = true;
    };
} // namespace Axle
