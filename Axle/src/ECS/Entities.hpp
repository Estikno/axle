#pragma once

#include "axpch.hpp"

#include "Core/Core.hpp"
#include "Core/Types.hpp"
#include "Core/Logger/Log.hpp"
#include "Core/Error/Panic.hpp"
#include "ComponentArray.hpp"

namespace Axle {
	using EntityID = u64;
	using ComponentType = u8;

	constexpr ComponentType MAX_COMPONENTS = 64;
	constexpr EntityID MAX_ENTITIES = 10000;

	using ComponentMask = std::bitset<MAX_COMPONENTS>;

	class Entities {
	public:
		Entities();

		/**
		* Registers a component for later use in entities
		*/
		template<typename T>
		void RegisterComponent();

		/**
		* Creates a new entity and returns a reference to the entities
		* class so that you can add components to it via the WithComponent method.
		*
		* @returns A reference to the entities for immediate use
		*/
		Entities& CreateEntity();

		/**
		* Adds a component to the entity that is currently being created.
		*
		* @param component Pointer to the component to be added. The added pointer will be managed by this class.
		*
		* @returns A reference to the entities class so that you can chain calls to this method.
		*/
		template<typename T>
		Entities& WithComponent(T component);

		/**
		* Adds a component to the entity with the given ID.
		*
		* @param id The ID of the entity to add the component to.
		* @param component Pointer to the component to be added. The added pointer will be managed by this class.
		*/
		template<typename T>
		void Add(EntityID id, T component);

		/**
		* Deletes a component of type T from the entity with the given ID.
		*
		* @param id The ID of the entity to remove the component from.
		*/
		template<typename T>
		void Remove(EntityID id);

		/**
		* Deletes an entity and all its components.
		*
		* @param id The ID of the entity to be deleted.
		*/
		void DeleteEntity(EntityID id);

		/**
		* Gets the id of the last created entity.
		*
		* @returns The ID of the last created entity.
		*/
		inline EntityID GetLastCreatedEntity() const noexcept {
			return m_InsertingIntoIndex;
		}

		/**
		* Checks if the entity with the given ID has a component of type T.
		*
		* @returns True if the entity has the component, false otherwise.
		*/
		template<typename T>
		inline bool Has(EntityID id) {
			return GetComponentBit<T>(GetMask(id));
		}

		/**
		* Checks if the entity with the given ID has all the components of the given types.
		*
		* @returns True if the entity has all the components, false otherwise.
		*/
		template<typename... Ts>
		inline bool HasAll(EntityID id) {
			return (Has<Ts>(id) && ...);
		}

		/**
		* Checks if the entity with the given ID has at least one component of the given types.
		*
		* @returns True if the entity has at least one of the components, false otherwise.
		*/
		template<typename... Ts>
		inline bool HasAny(EntityID id) {
			return (Has<Ts>(id) || ...);
		}

	private:
		template<typename T>
		ComponentType GetComponentType();

		template<typename T>
		void SetComponentBit(ComponentMask& mask, bool val) {
			size_t bitPos = GetComponentType<T>();
			mask.set(bitPos, val);
		}

		template<typename T>
		bool GetComponentBit(ComponentMask& mask) {
			size_t bitPos = GetComponentType<T>();
			return mask[bitPos];
		}

		inline ComponentMask& GetMask(EntityID id) {
			AX_ASSERT(m_LivingEntityCount < MAX_ENTITIES, "Entity {0} is out of bounce, the maximum id allowed is: {1}.", id, MAX_ENTITIES - 1);
			return m_EntityMasks.at(id);
		}

		template<typename T>
		inline bool IsComponentRegistered() {
			return m_ComponentTypes.find(std::type_index(typeid(T)) != m_ComponentTypes.end();
		}

		template<typename T>
		ComponentArray<T>& GetComponentArray() {
			std::type_index id = std::type_index(typeid(T));

			AX_ASSERT(IsComponentRegistered<T>(), "Component {0} has not been registered before use.", id.name());

			return *(static_cast<ComponentArray<T>*>(m_ComponentArrays.at(id).get()));
		}

		// A hasmap containing the components for every entity.
		//
		// The type_index  is used to identify the type of the component, and the vector
		// contains the actual components.
		//std::unordered_map<std::type_index, std::vector<std::shared_ptr<void>>> m_Components;
		std::unordered_map<std::type_index, std::unique_ptr<IComponentArray>> m_ComponentArrays;

		ComponentType m_NextComponentType;

		// The bitmasks of every registered component
		//
		// The bitmsk of every component is a bit shifted an adition than the previous component.
		// For example, the first component is `0001` and the second component is `0010`.
		//std::unordered_map<std::type_index, ComponentMask> m_ComponentMasks;
		std::unordered_map<std::type_index, ComponentType> m_ComponentTypes;

		/// A vector of bit masks for every entity.
		///
		/// The bit mask is used to store which components are enabled for a given
		/// entity.
		///
		/// For example, if the entity has the first and third registered component, the bit set will be `101`.
		std::array<ComponentMask, MAX_ENTITIES> m_EntityMasks;

		/// The index of the entity that is being inserted into.
		///
		/// This is used to keep track of which entity is being inserted into when
		/// creating and inserting components in it with the 'WithComponent' method.
		EntityID m_InsertingIntoIndex = 0;

		std::priority_queue<EntityID, std::vector<EntityID>, std::greater<EntityID>> m_AvailableEntities;
		EntityID m_LivingEntityCount = 0;
	};
}
