#pragma once

#include "axpch.hpp"

#include "Core/Core.hpp"

namespace Axle {
    class AXLE_API Layer {
    public:
        Layer(const std::string& name = "Layer");
        virtual ~Layer() = default;

        /**
         * Called when the layer is attached to the stack.
         * */
        virtual void OnAttach() = 0;

        /**
         * Called a fixed amount of times per second.
         * It's recommended to do all physics and logic updates here.
         * */
        virtual void OnUpdate() = 0;

        /**
         * Called when the layer is dettached from the stack.
         * */
        virtual void OnDettach() = 0;

        /**
         * Called when the layer is attached to the stack, but in the render thread.
         * All OpenGL calls must be done here.
         * */
        virtual void OnAttachRender() = 0;

        /**
         * Called every frame in the render thread.
         * All OpenGL calls must be done here. */
        virtual void OnRender() = 0;

        /**
         * Called when the layer is dettached from the stack, but in the render thread.
         * All OpenGL calls must be done here.
         * */
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
