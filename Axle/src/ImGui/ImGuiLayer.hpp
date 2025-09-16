#pragma once

#include "Core/Layer/Layer.hpp"
#include "Core/Core.hpp"
#include "Core/Types.hpp"

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
    };
} // namespace Axle
