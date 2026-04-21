#include "axpch.hpp"

#include "Core/Core.hpp"

#include "Core/Types.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Error/Panic.hpp"

#include "ECS.hpp"
#include "Other/CustomTypes/SparseSet.hpp"

namespace Axle {
    std::unique_ptr<ECS> ECS::m_ECS;

    ECS::ECS() {
        for (EntityID entity = 0; entity < MAX_ENTITIES; ++entity) {
            m_AvailableEntities.push(entity);
        }
    }

    void ECS::Init() {
        if (m_ECS != nullptr) {
            AX_CORE_WARN("Init method of the ECS has been called a second time. IGNORING");
            return;
        }

        m_ECS = std::make_unique<ECS>();

        AX_CORE_INFO("ECS initialized...");
    }

    void ECS::ShutDown() {
        m_ECS.reset();
        AX_CORE_INFO("ECS deleted...");
    }

    ECS& ECS::CreateEntity() {
        AX_ENSURE(m_LivingEntityCount < MAX_ENTITIES,
                  "Cannot create more entities than the maximum allowed: {0}.",
                  MAX_ENTITIES);

        // Take the smallest available entity ID
        EntityID id = m_AvailableEntities.top();
        m_AvailableEntities.pop();

        m_LivingEntities.at(id) = true;

        m_InsertingIntoIndex = id;
        m_LivingEntityCount++;

        return *this;

        AX_CORE_TRACE("Entity {0} has been created.", id);
    }

    void ECS::DeleteEntity(EntityID id) {
        AX_ASSERT(id < MAX_ENTITIES, "Entity ID {0} is out of bounds. Maximum ID is {1}.", id, MAX_ENTITIES - 1);
        AX_ASSERT(m_LivingEntities.at(id), "Entity ID {0} is not alive.", id);

        if (id >= MAX_ENTITIES) {
            AX_CORE_WARN("Entity {0} hasn't been removed it's out of bounds.", id);
            return;
        }

        if (!m_LivingEntities.at(id)) {
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

        m_LivingEntities.at(id) = false;

        // Put the destroyed ID back to the queue for future use
        m_AvailableEntities.push(id);
        m_LivingEntityCount--;

        AX_CORE_TRACE("Entity {0} has been deleted.", id);
    }
} // namespace Axle
