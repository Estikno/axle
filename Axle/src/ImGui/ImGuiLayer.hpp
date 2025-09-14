#pragma once

#include "Core/Layer/Layer.hpp"
#include "Core/Core.hpp"

namespace Axle {
    class ImGuiLayer : public Layer {
    public:
        ImGuiLayer();
        ~ImGuiLayer() override = default;

        void OnAttach() override;
        void OnUpdate() override;
        void OnDettach() override;

        void OnAttachRender() override;
        void OnRender() override;
        void OnDettachRender() override;
    };
} // namespace Axle
