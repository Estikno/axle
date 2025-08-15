#include "axpch.hpp"

#include "Core/Core.hpp"

#include "Core/Types.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Error/Panic.hpp"

#include "Systems.hpp"
#include "ECS.hpp"

namespace Axle {
    void Systems::Update(ECS& entities) {
        for (std::function<void(ECS&)>& system : m_Systems) {
            system(entities);
        }
    }
} // namespace Axle
