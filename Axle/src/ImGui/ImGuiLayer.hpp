#pragma once

#include "Core/Layer/Layer.hpp"
#include "Core/Core.hpp"

namespace Axle {
    class ImGuiLayer : public Layer {
    public:
        ImGuiLayer();
        ~ImGuiLayer() override;

        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate() override;
    };
} // namespace Axle
