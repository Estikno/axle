#include "axpch.hpp"

#include "Core/Core.hpp"

#include "Core/Types.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Error/Panic.hpp"

#include "Systems.hpp"
#include "Entities.hpp"

namespace Axle {
    void Systems::Update(Entities& entities) {
        for (std::function<void(Entities&)>& system : m_Systems) {
            system(entities);
        }
    }
} // namespace Axle
