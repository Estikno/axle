#include "axpch.hpp"

#include "Core/Types.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Error/Panic.hpp"

#include "Entities.hpp"

namespace Axle {
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

		if (component == nullptr) {
			AX_CORE_ERROR("Cannot add a null component of type {0} to entity {1}.", typeID.name(), id);
			Panic("Cannot add a null component of type {} to entity {}.", typeID.name(), id);
		}

		if (!IsComponentRegistered<T>()) {
			AX_CORE_ERROR("Component of type {0} is not registered.", typeID.name());
			Panic("Component of type {} is not registered.", typeID.name());
		}

		if (id >= m_EntityMasks.size()) {
			AX_CORE_ERROR("Entity ID {0} is out of bounds. Maximum ID is {1}.", id, m_EntityMasks.size() - 1);
			Panic("Entity ID {} is out of bounds. Maximum ID is {}.", id, m_EntityMasks.size() - 1);
		}

		ComponentMask& mask = GetComponentMask<T>();
		m_EntityMasks.at(id) |= *mask;

		std::vector<std::shared_ptr<void>>& components = m_Components.at(typeID);
		components.at(id) = std::static_pointer_cast<void>(std::shared_ptr<T>(component));
	}

	template<typename T>
	void Entities::Remove(EntityID id) {
		if (!IsComponentRegistered<T>()) {
			AX_CORE_ERROR("Component of type {0} is not registered.", typeID.name());
			Panic("Component of type {} is not registered.", typeID.name());
		}

		if (id >= m_EntityMasks.size()) {
			AX_CORE_ERROR("Entity ID {0} is out of bounds. Maximum ID is {1}.", id, m_EntityMasks.size() - 1);
			Panic("Entity ID {} is out of bounds. Maximum ID is {}.", id, m_EntityMasks.size() - 1);
		}

		std::type_index typeID = std::type_index(typeid(T));
		ComponentMask& mask = GetComponentMask<T>();

		if (Has<T>(id)) {
			m_EntityMasks.at(id) ^= *mask;
		}
		else {
			AX_CORE_WARN("Tried deleting component {0} from entity {1} which does not have it.", typeID.name(), id);
		}
	}

	void Entities::DeleteEntity(EntityID id) {
		if (id >= m_EntityMasks.size()) {
			AX_CORE_ERROR("Entity ID {0} is out of bounds. Maximum ID is {1}.", id, m_EntityMasks.size() - 1);
			Panic("Entity ID {} is out of bounds. Maximum ID is {}.", id, m_EntityMasks.size() - 1);
		}

		m_EntityMasks.at(id).reset();
	}
}