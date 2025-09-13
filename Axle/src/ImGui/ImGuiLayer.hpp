#pragma once

#include "Core/Layer/Layer.hpp"
#include "Core/Core.hpp"

namespace Axle {
    class ImGuiLayer : public Layer {
    public:
        ImGuiLayer();
        ~ImGuiLayer() override = default;

        void OnAttach() override;
        void OnDettach() override;
        void OnUpdate() override;
    };
} // namespace Axle
