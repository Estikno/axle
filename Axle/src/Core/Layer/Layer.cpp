#include "axpch.hpp"

#include "Layer.hpp"
#include "Core/Logger/Log.hpp"

namespace Axle {
    Layer::Layer(const std::string& name)
        : m_DebugName(name) {}

    void Layer::OnDettach() {}
} // namespace Axle
