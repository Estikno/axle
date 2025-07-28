#pragma once

#include "axpch.hpp"

#include "Core/Core.hpp"
#include "Core/Types.hpp"
#include "Core/Logger/Log.hpp"

namespace Axle {
	struct ResourceEntry {
		std::any ptr;
		std::function<void()> deleter;
	};

	class Resources {
	public:
		Resources() = default;
		AXLE_TEST_API ~Resources();

		template<typename T>
		void Add(const T* resource) = delete; // forbid const pointers

		/**
		* Add a global resource to the resource manager.
		* 
		* @param resource Pointer to the resource to be added. The added pointer will be managed
		* by the resource manager, you must not delete it.
		*/
		template<typename T>
		void Add(T* resource);

		/**
		* Gets a resource of type T from the resource manager.
		* 
		* @returns Pointer to the resource of type T if it exists, otherwise nullptr.
		*/
		template<typename T>
		T* Get();

		/**
		* Removes a resource of type T from the resource manager.
		*/
		template<typename T>
		void Remove();

		/**
		* Checks if a resource of type T exists in the resource manager.
		* 
		* @returns True if the resource exists, otherwise false.
		*/
		template<typename T>
		bool Contains() const noexcept;

#ifdef AXLE_TESTING
		/// This function is only available for testing purposes.
		std::unordered_map<std::type_index, ResourceEntry>& GetData() {
			return m_Data;
		}
#endif // AXLE_TESTING

	private:
		/// The hash map that stores the resources, where the key is the type index of the resource type.
		/// This means that there can't be two resources of the same type stored at the same time.
		std::unordered_map<std::type_index, ResourceEntry> m_Data;
	};
}
