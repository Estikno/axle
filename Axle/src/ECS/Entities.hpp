#pragma once

#include "axpch.hpp"

#include "Core/Core.hpp"
#include "Core/Types.hpp"
#include "Core/Logger/Log.hpp"
#include "Math/Mathf.hpp"

namespace Axle {
	constexpr size_t MAX_COMPONENTS = 64;
	constexpr EntityID MAX_ENTITIES = 10000;

	using EntityID = u64;
	using ComponentMask = std::bitset<MAX_COMPONENTS>;

	class Entities {
	public:
		Entities() = default;

		template<typename T>
		void RegisterComponent();

		Entities CreateEntity();

		template<typename T>
		Entities WithComponent(T* component);

		template<typename T>
		void Add(EntityID id, T* component);

		template<typename T>
		void Remove(EntityID id);

		void DeleteEntity(EntityID id);

	private:
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
