#pragma once

#include "axpch.hpp"

#include "Entities.hpp"
#include "Core/Core.hpp"
#include "Core/Types.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Error/Panic.hpp"

namespace Axle {
	class IComponentArray {
	public:
		virtual ~IComponentArray() = default;
		virtual void EntityDestroyed(EntityID id) = 0;
	};

	template<typename T>
	class ComponentArray : public IComponentArray {
	public:
		void InsertData(EntityID id, T component) {
			AX_ASSERT(m_EntityToIndexMap.find(id) == m_EntityToIndexMap.end(), "Entity {0} already has a component of type {1}.", id, typeid(T).name());

			// Put an entry at the end and update the maps
			size_t newIndex = m_Size;
			m_EntityToIndexMap[id] = newIndex;
			m_IndexToEntityMap[newIndex] = id;
			m_ComponentArray.at(newIndex) = component;

			m_Size++;
		}

		void RemoveData(EntityID id) {
			AX_ASSERT(m_EntityToIndexMap.find(id) != m_EntityToIndexMap.end(), "Trying to remove a non existen component from entity {0}", id);

			// Copy element at the end into the deleted element's place to mantain density
			size_t indexOfRemovedEntity = m_EntityToIndexMap.at(id);
			size_t indexOfLastElement = m_Size - 1;
			m_ComponentArray.at(indexOfRemovedEntity) = m_ComponentArray.at(indexOfLastElement);

			// Update the maps of this change
			EntityID entityOfLastElement = m_IndexToEntityMap.at(indexOfLastElement);
			m_EntityToIndexMap.at(entityOfLastElement) = indexOfRemovedEntity;
			m_IndexToEntityMap.at(indexOfRemovedEntity) = entityOfLastElement;

			// Erase the remaining data
			m_EntityToIndexMap.erase(id);
			m_IndexToEntityMap.erase(indexOfLastElement);

			m_Size--;
		}

		T& GetData(EntityID id) {
			AX_ASSERT(m_EntityToIndexMap.find(id) != m_EntityToIndexMap.end(), "Trying to retieve a non existent component of type: ", typeid(T).name());

			return m_ComponentArray.at(m_EntityToIndexMap.at(id));
		}

		void EntityDestroyed(EntityID id) override {
			if (m_EntityToIndexMap.find(id) != m_EntityToIndexMap.end()) {
				RemoveData(id);
			}
		}
	private:
		// A packed array of components of type T
		// Each entity has a unique component
		std::array<T, MAX_ENTITIES> m_ComponentArray;

		// Map from an entity ID to its index in the component array
		std::unordered_map<EntityID, size_t> m_EntityToIndexMap;

		// Map from an index in the component array to its entity ID
		std::unordered_map<size_t, EntityID> m_IndexToEntityMap;

		// Total size of valid entries in the array
		size_t m_Size = 0;
	};
}
