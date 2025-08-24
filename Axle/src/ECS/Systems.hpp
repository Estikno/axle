#pragma once

#include "axpch.hpp"

#include "Core/Types.hpp"
#include "Core/Core.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Error/Panic.hpp"
#include "Other/CustomTypes/SparseSet.hpp"

#include "ECS.hpp"

namespace Axle {
    class AXLE_TEST_API Systems {
    public:
        /**
         * Excecutes all the systems added to the systems manager.
         * */
        void Update(ECS& entities);

        /**
         * Adds a system to the systems manager. The system can simply be a lambda function.
         *
         * The function must take at least a refernce to the components it wants to operate on. And optionally
         * the EntityID of the current entity that is modifying, this paramenter should go before the components.
         * */
        template <typename... Components, typename Func>
        void Add(Func&& func) {
            // We create a wrapper around each function so that we can call it with the ECS reference
            // This simplifies the storing process.

            // Detects if the lambda has an EntityID parameter or not
            if constexpr (std::is_invocable_v<Func, EntityID, Components&...>) {
                // The case: lambda with EntityID
                m_Systems.emplace_back([func = std::forward<Func>(func)](ECS& entities) {
                    View<Components...> view(entities);
                    std::vector<EntityID> entitiesList = view.GetEntities();
                    std::vector<std::tuple<Components&...>> componentsList = view.GetComponents();

                    for (size_t i = 0; i < entitiesList.size(); ++i) {
                        std::apply([&](Components&... comps) { func(entitiesList[i], comps...); }, componentsList[i]);
                    }
                });
            } else {
                // The case: lambda without EntityID
                m_Systems.emplace_back([func = std::forward<Func>(func)](ECS& entities) {
                    View<Components...> view(entities);
                    auto componentsList = view.GetComponents();

                    for (auto& compsTuple : componentsList) {
                        std::apply(func, compsTuple);
                    }
                });
            }
        }

    private:
        /// A vector to store all the system wrappers to the real functions
        std::vector<std::function<void(ECS&)>> m_Systems;
    };
} // namespace Axle
