#pragma once

#include "axpch.hpp"

#include "Core/Core.hpp"
#include "Core/Events/Event.hpp"

namespace Axle {
    class AXLE_API Layer {
    public:
        Layer(const std::string& name = "Layer");
        virtual ~Layer();

        virtual void OnAttach() = 0;
        virtual void OnDetach() = 0;
        virtual void OnUpdate() = 0;
        // virtual void OnEvent(Event* event) = 0;

        inline const std::string& GetName() const {
            return m_DebugName;
        }

    protected:
        std::string m_DebugName;
    };
} // namespace Axle
