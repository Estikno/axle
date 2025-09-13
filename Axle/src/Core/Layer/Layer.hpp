#pragma once

#include "axpch.hpp"

#include "Core/Core.hpp"

namespace Axle {
    class AXLE_API Layer {
    public:
        Layer(const std::string& name = "Layer");
        virtual ~Layer() = default;

        virtual void OnAttach() = 0;
        virtual void OnUpdate() = 0;
        virtual void OnDettach() = 0;

        virtual void OnAttachRender() = 0;
        virtual void OnRender() = 0;
        virtual void OnDettachRender() = 0;

        inline const std::string& GetName() const {
            return m_DebugName;
        }

    protected:
        std::string m_DebugName;

        /// The indexes of the events the layer is subscribed to.
        std::vector<size_t> m_EventSubscribeIndexes;
    };
} // namespace Axle
