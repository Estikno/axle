#pragma once

#include "axpch.hpp"

#include "Core/Types.hpp"
#include "Core/Core.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Error/Panic.hpp"
#include "Other/Helpers/SparseSet.hpp"

#include "Entities.hpp"

namespace Axle {
    class AXLE_TEST_API Systems {
    public:
        void Update(Entities& entities);

        template <typename... Components, typename Func>
        void Add(Func&& func) {
            m_Systems.emplace_back([func = std::forward<Func>(func)](Entities& entities) {
                View<Components...> view(entities);
                view.ForEach(func);
            });
        }

    private:
        std::vector<std::function<void(Entities&)>> m_Systems;
    };
} // namespace Axle
