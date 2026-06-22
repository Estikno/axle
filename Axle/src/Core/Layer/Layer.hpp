#pragma once

#include "axpch.hpp"

#include "Core/Core.hpp"
#include "Core/Types.hpp"
#include "Core/Events/Event.hpp"
#include "Other/CustomTypes/TripleBuffer.hpp"

namespace Axle {
    class AXLE_API Layer {
    public:
        Layer(const std::string& name = "Layer");
        virtual ~Layer() = default;

        /**
         * Called when the layer is attached to the stack.
         *
         * Important: It's not guaranteed that this method is always going to be excecuted by the same thread
         * */
        virtual void OnAttach() = 0;

        /**
         * Called a fixed amount of times per second.
         * It's recommended to do all physics and logic updates here.
         *
         * Important: It is not guaranteed that all OnUpdate methods will be excecuted by the same thread.
         * */
        virtual void OnUpdate(f64 FixedDeltaTime) = 0;

        // TODO: Finish the implementation
        /**
         * Publishes the back buffer written in the OnUpdate method so the render thread can safely read the
         * changes.
         *
         * Important: It's not guaranteed that this method is always going to be excecuted by the same thread
         * */
        virtual void CommitSnapshot() {}

        /**
         * Called when the layer is dettached from the stack.
         *
         * Important: It's not guaranteed that this method is always going to be excecuted by the same thread
         * */
        virtual void OnDettach();

        /**
         * Called when the layer is attached to the stack, but in the render thread.
         * All OpenGL calls must be done here.
         *
         * Important: It is guaranteed that this method will be called only by the render thread
         * */
        virtual void OnAttachRender() = 0;

        /**
         * Called every frame in the render thread.
         * All OpenGL calls must be done here.
         *
         * Important: It is guaranteed that this method will be called only by the render thread
         * */
        virtual void OnRender(f64 DeltaTime) = 0;

        /**
         * Called when the layer is dettached from the stack, but in the render thread.
         * All OpenGL calls must be done here.
         *
         * Important: It is guaranteed that this method will be called only by the render thread
         * */
        virtual void OnDettachRender() = 0;

        /**
         * Called when an event is passed to the layer
         *
         * Important: It's not guaranteed that this method is always going to be excecuted by the same thread
         * */
        virtual void OnEvent(Event& event) = 0;

        inline const std::string& GetName() const {
            return m_DebugName;
        }

    protected:
        std::string m_DebugName;
    };
} // namespace Axle
