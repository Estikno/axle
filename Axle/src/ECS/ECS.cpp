#include "axpch.hpp"

#include "Core/Core.hpp"

#include "Core/Types.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Error/Panic.hpp"

#include "ECS.hpp"
#include "Other/CustomTypes/SparseSet.hpp"

namespace Axle {
    std::unique_ptr<ECS> ECS::s_ECS;

    void ECS::Init() {
        if (s_ECS != nullptr) {
            AX_CORE_WARN("Init method of the ECS has been called a second time. IGNORING");
            return;
        }

        s_ECS = std::make_unique<ECS>();
        AX_CORE_INFO("ECS initialized...");
    }

    void ECS::ShutDown() {
        s_ECS.reset();
        AX_CORE_INFO("ECS deleted...");
    }

    EntityID ECS::CreateEntity() {
        std::scoped_lock lock(m_EntitiesMutex);
        AX_ENSURE(m_LivingEntityCount < MAX_ENTITIES,
                  "Cannot create more entities than the maximum allowed: {0}.",
                  MAX_ENTITIES);

        // Take the smallest available entity ID
        EntityID id;
        if (m_AvailableEntities.empty())
            id = m_LargestAvailableEntityID++;
        else {
            id = m_AvailableEntities.top();
            m_AvailableEntities.pop();
        }

        m_LivingEntities.set(id, true);
        m_LivingEntityCount++;

        AX_CORE_TRACE("Entity {0} has been created.", id);
        return id;
    }

    void ECS::DeleteEntity(EntityID id) {
        std::scoped_lock lock(m_EntitiesMutex, m_ComponentsMutex);
        DeleteEntityUnsafe(id);
    }

    void ECS::DeleteEntityUnsafe(EntityID id) {
        AX_ASSERT(id < MAX_ENTITIES, "Entity ID {0} is out of bounds. Maximum ID is {1}.", id, MAX_ENTITIES - 1);
        AX_ASSERT(m_LivingEntities[id], "Entity ID {0} is not alive.", id);

        if (id >= MAX_ENTITIES) {
            AX_CORE_WARN("Entity {0} hasn't been removed it's out of bounds.", id);
            return;
        }

        if (!m_LivingEntities[id]) {
            AX_CORE_WARN("Entity {0} hasn't been removed because the entity is not alive.", id);
            return;
        }

        for (auto const& [typeID, componentArray] : m_ComponentArrays) {
            // Notify the sparse sets that the entity has been destroyed
            // Call with no panic because we don't care if it fails and does nothing
            componentArray->RemoveNoPanic(id);
        }

        // Invalidate the mask of the entity
        m_EntityMasks.at(id).reset();

        m_LivingEntities.set(id, false);

        // Put the destroyed ID back to the queue for future use
        m_AvailableEntities.push(id);
        m_LivingEntityCount--;

        AX_CORE_TRACE("Entity {0} has been deleted.", id);
    }

} // namespace Axle
