#include "axpch.hpp"

#include "Layer.hpp"
#include "Core/Events/EventHandler.hpp"
#include "Core/Logger/Log.hpp"

namespace Axle {
    Layer::Layer(const std::string& name)
        : m_DebugName(name) {}

    void Layer::OnDettach() {
        for (size_t index : m_EventSubscribeIndexes) {
            EventHandler::GetInstance().Unsubscribe(index);
        }
    }
} // namespace Axle
