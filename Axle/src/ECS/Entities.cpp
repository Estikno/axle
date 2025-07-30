#include "axpch.hpp"

#include "Core/Types.hpp"
#include "Core/Logger/Log.hpp"

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

		m_Components.insert({ typeID, std::vector<std::shared_ptr<void>>() });
		m_ComponentMasks.insert({ typeID, newMask });
	}

	Entities& Entities::CreateEntity() {
		for (size_t i = 0; i < m_EntityMasks.size(); i++) {
			if (m_EntityMasks[i].none()) {
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
		std::type_index typeID = std::type_index(typeid(T));
		EntityID id = m_InsertingIntoIndex;

		if (m_Components.find(typeID) == m_Components.end()) {
			AX_CORE_ERROR("Component of type {0} is not registered.", typeID.name());
			throw std::runtime_error("Component not registered");
		}

		std::vector<std::shared_ptr<void>>& components = m_Components[typeID];
	}
}