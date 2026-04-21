#pragma once

#include "Core/Events/Event.hpp"
#include "Core/Layer/Layer.hpp"
#include "Core/Types.hpp"

#include "Debug/Console.hpp"
#include "Debug/ECSEditor.hpp"

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

    private:
        // Renderer variables
        Debug::DebugConsole m_Console;
        bool m_OpenOverlay = true;
        Debug::ECSEditor m_ECSEditor;
    };
} // namespace Axle
