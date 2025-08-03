#include "axpch.hpp"

#include "Core/Core.hpp"

#include "Core/Types.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Error/Panic.hpp"

#include "Entities.hpp"
#include "ComponentArray.hpp"

namespace Axle {
	Entities::Entities() {
		for (EntityID entity = 0; entity < MAX_ENTITIES; entity++) {
			m_AvailableEntities.push(entity);
		}
	}

	template<typename T>
	void Entities::RegisterComponent() {
		ComponentType typeID = GetComponentType<T>();
		AX_ASSERT(typeID < MAX_COMPONENTS, "Too many components registered.");

		if (m_ComponentArrays.find(typeID) != m_ComponentArrays.end()) {
			AX_CORE_WARN("Component of type {0} is already registered.", typeid(T).name());
			return;
		}

		m_ComponentArrays.insert({ typeID, std::make_unique<ComponentArray<T>>() });
	}

	Entities& Entities::CreateEntity() {
		AX_ASSERT(m_LivingEntityCount < MAX_ENTITIES, "Cannot create more entities than the maximum allowed: {0}.", MAX_ENTITIES);

		// Take the smallest available entity ID
		EntityID id = m_AvailableEntities.top();
		m_AvailableEntities.pop();

		m_InsertingIntoIndex = id;
		m_LivingEntityCount++;

		return *this;
	}

	template<typename T>
	Entities& Entities::WithComponent(T component) {
		Add<T>(m_InsertingIntoIndex, component);
		return *this;
	}

	template<typename T>
	void Entities::Add(EntityID id, T component) {
		AX_ASSERT(IsComponentRegistered<T>(), "Component of type {0} is not registered.", typeid(T).name());
		AX_ASSERT(id < MAX_ENTITIES, "Entity ID {0} is out of bounds. Maximum ID is {1}.", id, MAX_ENTITIES - 1);

		ComponentArray<T>& array = GetComponentArray<T>();
		array.Add(id, component);

		ComponentMask& entityMask = GetMask(id);
		SetComponentBit<T>(entityMask, true);
	}

	template<typename T>
	void Entities::Remove(EntityID id) {
		AX_ASSERT(IsComponentRegistered<T>(), "Component of type {0} is not registered.", typeid(T).name());
		AX_ASSERT(id < MAX_ENTITIES, "Entity ID {0} is out of bounds. Maximum ID is {1}.", id, MAX_ENTITIES - 1);

		ComponentArray<T>& array = GetComponentArray<T>();
		array.Remove(id);

		ComponentMask& entityMask = GetMask(id);
		SetComponentBit<T>(entityMask, false);
	}

	void Entities::DeleteEntity(EntityID id) {
		AX_ASSERT(id < MAX_ENTITIES, "Entity ID {0} is out of bounds. Maximum ID is {1}.", id, MAX_ENTITIES - 1);

		for (auto const& [typeID, componentArray] : m_ComponentArrays) {
			// Call the EntityDestroyed method of the component array
			componentArray->EntityDestroyed(id);
		}

		// Invalidate the mask of the entity
		m_EntityMasks.at(id).reset();

		// Put the destroyed ID back to the queue
		m_AvailableEntities.push(id);
		m_LivingEntityCount--;
	}

	template<typename T>
	T& Entities::Get(EntityID id) {
		AX_ASSERT(IsComponentRegistered<T>(), "Component of type {0} is not registered.", typeid(T).name());
		AX_ASSERT(id < MAX_ENTITIES, "Entity ID {0} is out of bounds. Maximum ID is {1}.", id, MAX_ENTITIES - 1);

		return GetComponentArray<T>().Get(id);
	}

#ifdef AXLE_TESTING
	template AXLE_TEST_API void Entities::RegisterComponent<Position>();
	template AXLE_TEST_API void Entities::RegisterComponent<Velocity>();
	template AXLE_TEST_API Entities& Entities::WithComponent<Position>(Position);
	template AXLE_TEST_API Entities& Entities::WithComponent<Velocity>(Velocity);
	template AXLE_TEST_API void Entities::Add<Position>(EntityID, Position);
	template AXLE_TEST_API void Entities::Add<Velocity>(EntityID, Velocity);
	template AXLE_TEST_API void Entities::Remove<Position>(EntityID);
	template AXLE_TEST_API void Entities::Remove<Velocity>(EntityID);
	template AXLE_TEST_API bool Entities::Has<Position>(EntityID);
	template AXLE_TEST_API bool Entities::Has<Velocity>(EntityID);
	template AXLE_TEST_API bool Entities::HasAll<Position, Velocity>(EntityID);
	template AXLE_TEST_API bool Entities::HasAny<Position, Velocity>(EntityID);
	template AXLE_TEST_API class ComponentArray<Position>;
	template AXLE_TEST_API class ComponentArray<Velocity>;
	template AXLE_TEST_API Position& Entities::Get<Position>(EntityID);
	template AXLE_TEST_API Velocity& Entities::Get<Velocity>(EntityID);
#endif // AXLE_TESTING

}