#pragma once

#include "axpch.hpp"

#include "Core/Core.hpp"
#include "Core/Types.hpp"
#include "Core/Logger/Log.hpp"

namespace Axle {
	class Resources {
	public:
		Resources() = default;
		~Resources() = default;

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
		* Add a global resource to the resource manager but via a shared pointer.
		* 
		* @param resource Shared pointer to the resource to be added. The added pointer will be managed
		* by the resource manager, you must not delete the underlying pointer.
		*/
		template<typename T>
		void Add(std::shared_ptr<T> resource);

		/**
		* Gets a resource of type T from the resource manager.
		* The returned pointer shall not be deleted, as it is managed by the resource manager.
		* 
		* If you get the pointer and then later delete the resource, the pointer will become invalid.
		* 
		* @returns Pointer to the resource of type T if it exists, otherwise nullptr.
		*/
		template<typename T>
		T* Get();

		/**
		* Gets a shared pointer to a resource of type T from the resource manager.
		* This is useful for resources that are shared across multiple components or systems and
		* you want to make sure that in that time the pointer is not deleted.
		* 
		* @returns Shared pointer to the resource of type T if it exists, otherwise nullptr.
		*/
		template<typename T>
		std::shared_ptr<T> GetShared();

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
		std::unordered_map<std::type_index, std::shared_ptr<void>>& GetDataTEST() {
			return m_Data;
		}
#endif // AXLE_TESTING

	private:
		/// The hash map that stores the resources, where the key is the type index of the resource type.
		/// This means that there can't be two resources of the same type stored at the same time.
		std::unordered_map<std::type_index, std::shared_ptr<void>> m_Data;
	};
}
