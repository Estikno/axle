#include "axpch.hpp"

#include "Core/Core.hpp"

#include "Core/Types.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Error/Panic.hpp"

#include "ECS.hpp"
#include "Other/CustomTypes/SparseSet.hpp"

namespace Axle {
    ECS::ECS() {
        for (EntityID entity = 0; entity < MAX_ENTITIES; ++entity) {
            m_AvailableEntities.push(entity);
        }
    }

    template <typename T>
    void ECS::RegisterComponent() {
        ComponentType typeID = GetComponentType<T>();
        AX_ENSURE(typeID < MAX_COMPONENTS, "Too many components registered. The maximum is {0}.", MAX_COMPONENTS);

        if (m_ComponentArrays.find(typeID) != m_ComponentArrays.end()) {
            AX_CORE_WARN("Component of type {0} is already registered.", typeid(T).name());
            return;
        }

        m_ComponentArrays.insert({typeID, std::make_unique<SparseSet<T>>()});

        AX_CORE_TRACE("Component {0} has been registered.", typeid(T).name());
    }

    ECS& ECS::CreateEntity() {
        AX_ENSURE(m_LivingEntityCount < MAX_ENTITIES,
                  "Cannot create more entities than the maximum allowed: {0}.",
                  MAX_ENTITIES);

        // Take the smallest available entity ID
        EntityID id = m_AvailableEntities.top();
        m_AvailableEntities.pop();

        m_InsertingIntoIndex = id;
        m_LivingEntityCount++;

        return *this;

        AX_CORE_TRACE("Entity {0} has been created.", id);
    }

    template <typename T>
    ECS& ECS::WithComponent(T component) {
        Add<T>(m_InsertingIntoIndex, component);
        return *this;
    }

    template <typename T>
    void ECS::Add(EntityID id, T component) {
        AX_ASSERT(IsComponentRegistered<T>(), "Component of type {0} is not registered.", typeid(T).name());
        AX_ASSERT(id < MAX_ENTITIES, "Entity ID {0} is out of bounds. Maximum ID is {1}.", id, MAX_ENTITIES - 1);

        SparseSet<T>& array = GetComponentArray<T>();
        array.Add(id, component);

        ComponentMask& entityMask = GetMask(id);
        SetComponentBit<T>(entityMask, true);

        AX_CORE_TRACE("Component {0} has been added to entity {1}.", typeid(T).name(), id);
    }

    template <typename T>
    void ECS::Remove(EntityID id) {
        AX_ASSERT(IsComponentRegistered<T>(), "Component of type {0} is not registered.", typeid(T).name());
        AX_ASSERT(id < MAX_ENTITIES, "Entity ID {0} is out of bounds. Maximum ID is {1}.", id, MAX_ENTITIES - 1);

        SparseSet<T>& array = GetComponentArray<T>();
        array.Remove(id);

        ComponentMask& entityMask = GetMask(id);
        SetComponentBit<T>(entityMask, false);

        AX_CORE_TRACE("Component {0} has been removed from entity {1}.", typeid(T).name(), id);
    }

    void ECS::DeleteEntity(EntityID id) {
        AX_ASSERT(id < MAX_ENTITIES, "Entity ID {0} is out of bounds. Maximum ID is {1}.", id, MAX_ENTITIES - 1);

        for (auto const& [typeID, componentArray] : m_ComponentArrays) {
            // Notify the sparse sets that the entity has been destroyed
            // Call with no panic because we don't care if it fails and does nothing
            componentArray->RemoveNoPanic(id);
        }

        // Invalidate the mask of the entity
        m_EntityMasks.at(id).reset();

        // Put the destroyed ID back to the queue
        m_AvailableEntities.push(id);
        m_LivingEntityCount--;

        AX_CORE_TRACE("Entity {0} has been deleted.", id);
    }

    template <typename T>
    T& ECS::Get(EntityID id) {
        AX_ASSERT(IsComponentRegistered<T>(), "Component of type {0} is not registered.", typeid(T).name());
        AX_ASSERT(id < MAX_ENTITIES, "Entity ID {0} is out of bounds. Maximum ID is {1}.", id, MAX_ENTITIES - 1);

        return GetComponentArray<T>().Get(id);
    }

#ifdef AXLE_TESTING
    template AXLE_TEST_API void ECS::RegisterComponent<Position>();
    template AXLE_TEST_API void ECS::RegisterComponent<Velocity>();
    template AXLE_TEST_API ECS& ECS::WithComponent<Position>(Position);
    template AXLE_TEST_API ECS& ECS::WithComponent<Velocity>(Velocity);
    template AXLE_TEST_API void ECS::Add<Position>(EntityID, Position);
    template AXLE_TEST_API void ECS::Add<Velocity>(EntityID, Velocity);
    template AXLE_TEST_API void ECS::Remove<Position>(EntityID);
    template AXLE_TEST_API void ECS::Remove<Velocity>(EntityID);
    template AXLE_TEST_API bool ECS::Has<Position>(EntityID);
    template AXLE_TEST_API bool ECS::Has<Velocity>(EntityID);
    template AXLE_TEST_API bool ECS::HasAll<Position, Velocity>(EntityID);
    template AXLE_TEST_API bool ECS::HasAny<Position, Velocity>(EntityID);
    template AXLE_TEST_API Position& ECS::Get<Position>(EntityID);
    template AXLE_TEST_API Velocity& ECS::Get<Velocity>(EntityID);
    template AXLE_TEST_API class View<Position, Velocity>;
    template AXLE_TEST_API class View<Velocity>;
    template AXLE_TEST_API class View<Position>;
    template AXLE_TEST_API class SparseSet<Position>;
    template AXLE_TEST_API class SparseSet<Velocity>;
#endif // AXLE_TESTING

} // namespace Axle
