#pragma once

#include "axpch.hpp"

#include "Core/Core.hpp"
#include "Layer.hpp"

namespace Axle {
    class AXLE_API LayerStack {
    public:
        ~LayerStack();

        /**
         * Pushes a layer to the stack.
         * LAYERS are pushed below OVERLAYS.
         *
         * @param layer The layer to push.
         * */
        void PushLayer(Layer* layer);

        /**
         * Pushes an overlay to the stack.
         * An OVERLAY is always pushed on top of all layers.
         *
         * @param overlay The overlay to push.
         * */
        void PushOverlay(Layer* overlay);

        /**
         * Removes a layer from the stack.
         * The pointer is not deleted.
         *
         * @param layer The layer to remove.
         * */
        void PopLayer(Layer* layer);

        /**
         * Removes an overlay from the stack.
         * The pointer is not deleted.
         *
         * @param overlay The overlay to remove.
         * */
        void PopOverlay(Layer* overlay);

        std::vector<Layer*>::iterator begin() {
            return m_Layers.begin();
        }
        std::vector<Layer*>::iterator end() {
            return m_Layers.end();
        }
        std::vector<Layer*>::reverse_iterator rbegin() {
            return m_Layers.rbegin();
        }
        std::vector<Layer*>::reverse_iterator rend() {
            return m_Layers.rend();
        }

    private:
        std::vector<Layer*> m_Layers;
        size_t m_LayerInsert = 0;
    };
} // namespace Axle
