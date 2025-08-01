#include "axpch.hpp"

#include "Core/Core.hpp"

#include "Core/Types.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Error/Panic.hpp"

#include "Entities.hpp"

namespace Axle {
	Entities::Entities() {
		for (EntityID entity = 0; entity < MAX_ENTITIES; entity++) {
			m_AvailableEntities.push(entity);
		}
	}

	template<typename T>
	void Entities::RegisterComponent() {
		std::type_index typeID = std::type_index(typeid(T));

		if (m_Components.find(typeID) != m_Components.end()) {
			AX_CORE_WARN("Component of type {0} is already registered.", typeID.name());
			return;
		}

		ComponentMask newMask;
		newMask.set(m_ComponentMasks.size());

		m_Components.insert({ typeID, std::vector<std::shared_ptr<void>>(m_EntityMasks.size()) });
		m_ComponentMasks.insert({ typeID, newMask });
	}

	Entities& Entities::CreateEntity() {
		for (size_t i = 0; i < m_EntityMasks.size(); i++) {
			if (m_EntityMasks.at(i).none()) {
				m_InsertingIntoIndex = i;
				return *this;
			}
		}

		for (auto& [_, vec] : m_Components) {
			vec.push_back(nullptr);
		}

		m_EntityMasks.push_back(ComponentMask());
		m_InsertingIntoIndex = m_EntityMasks.size() - 1;

		return *this;
	}

	template<typename T>
	Entities& Entities::WithComponent(T* component) {
		Add<T>(m_InsertingIntoIndex, component);
		return *this;
	}

	template<typename T>
	void Entities::Add(EntityID id, T* component) {
		std::type_index typeID = std::type_index(typeid(T));

		AX_ASSERT(component != nullptr, "Cannot add a null component of type {0} to entity {1}", typeID.name(), id);
		AX_ASSERT(IsComponentRegistered<T>(), "Component of type {0} is not registered.", typeID.name());
		AX_ASSERT(id < m_EntityMasks.size(), "Entity ID {0} is out of bounds. Maximum ID is {1}.", id, m_EntityMasks.size() - 1);

		ComponentMask& mask = GetComponentMask<T>();
		m_EntityMasks.at(id) |= mask;

		std::vector<std::shared_ptr<void>>& components = m_Components.at(typeID);
		components.at(id) = std::static_pointer_cast<void>(std::shared_ptr<T>(component));
	}

	template<typename T>
	void Entities::Remove(EntityID id) {
		std::type_index typeID = std::type_index(typeid(T));

		AX_ASSERT(IsComponentRegistered<T>(), "Component of type {0} is not registered.", typeID.name());
		AX_ASSERT(id < m_EntityMasks.size(), "Entity ID {0} is out of bounds. Maximum ID is {1}.", id, m_EntityMasks.size() - 1);

		ComponentMask& mask = GetComponentMask<T>();

		if (Has<T>(id)) {
			m_EntityMasks.at(id) ^= mask;
		}
		else {
			AX_CORE_WARN("Tried deleting component {0} from entity {1} which does not have it.", typeID.name(), id);
		}
	}

	void Entities::DeleteEntity(EntityID id) {
		AX_ASSERT(id < m_EntityMasks.size(), "Entity ID {0} is out of bounds. Maximum ID is {1}.", id, m_EntityMasks.size() - 1);

		m_EntityMasks.at(id).reset();
	}

#ifdef AXLE_TESTING
	template AXLE_TEST_API Entities& Entities::WithComponent<Velocity>(Velocity*);
	template AXLE_TEST_API Entities& Entities::WithComponent<Position>(Position*);

	template AXLE_TEST_API void Entities::RegisterComponent<Position>();
	template AXLE_TEST_API void Entities::Add<Position>(EntityID, Position*);
	template AXLE_TEST_API void Entities::Remove<Position>(EntityID);
	template AXLE_TEST_API bool Entities::Has<Position>(EntityID);

	template AXLE_TEST_API void Entities::RegisterComponent<Velocity>();
	template AXLE_TEST_API void Entities::Add<Velocity>(EntityID, Velocity*);
	template AXLE_TEST_API void Entities::Remove<Velocity>(EntityID);
	template AXLE_TEST_API bool Entities::Has<Velocity>(EntityID);

	template AXLE_TEST_API bool Entities::HasAll<Position, Velocity>(EntityID);
	template AXLE_TEST_API bool Entities::HasAny<Position, Velocity>(EntityID);
#endif // AXLE_TESTING
}