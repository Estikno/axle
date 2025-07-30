#pragma once

#include "axpch.hpp"

#include "Core/Core.hpp"
#include "Core/Types.hpp"
#include "Core/Logger/Log.hpp"
#include "Math/Mathf.hpp"

namespace Axle {
	using EntityID = u64;

	constexpr size_t MAX_COMPONENTS = 64;
	constexpr EntityID MAX_ENTITIES = 10000;

	using ComponentMask = std::bitset<MAX_COMPONENTS>;

	class Entities {
	public:
		Entities() = default;

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
		Entities& WithComponent(T* component);

		/**
		* Adds a component to the entity with the given ID.
		* 
		* @param id The ID of the entity to add the component to.
		* @param component Pointer to the component to be added. The added pointer will be managed by this class.
		*/
		template<typename T>
		void Add(EntityID id, T* component);

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
			ComponentMask& mask = GetComponentMask<T>();
			return m_EntityMasks.at(id) & *mask == *mask;
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
		inline ComponentMask& GetComponentMask() {
			return m_ComponentMasks.at(std::type_index(typeid(T)));
		}

		/// A hasmap containing the components for every entity.
		///
		/// The type_index  is used to identify the type of the component, and the vector
		/// contains the actual components.
		std::unordered_map<std::type_index, std::vector<std::shared_ptr<void>>> m_Components;

		/// The bitmasks of every registered component
		///
		/// The bitmsk of every component is a bit shifted an adition than the previous component.
		/// For example, the first component is `0001` and the second component is `0010`.
		std::unordered_map<std::type_index, ComponentMask> m_ComponentMasks;

		/// A vector of bit masks for every entity.
		///
		/// The bit mask is used to store which components are enabled for a given
		/// entity.
		///
		/// For example, if the entity has the first and third registered component, the bit set will be `101`.
		std::vector<ComponentMask> m_EntityMasks;

		/// The index of the entity that is being inserted into.
		///
		/// This is used to keep track of which entity is being inserted into when
		/// creating and inserting components in it with the 'WithComponent' method.
		EntityID m_InsertingIntoIndex = 0;
	};
}
