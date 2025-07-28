#include "axpch.hpp"

#include "Core/Core.hpp"
#include "Core/Types.hpp"
#include "Core/Logger/Log.hpp"

#include "Resources.hpp"

namespace Axle {
	template<typename T>
	void Resources::Add(T* resource) {
		std::type_index id = std::type_index(typeid(T));

		if (resource == nullptr) {
			AX_CORE_ERROR("Cannot add a null resource to the resource manager.");
			return;
		}

		if (this->Contains<T>()) {
			AX_CORE_WARN("An element of the same type already exists in the resource manager. Overwriting it.");
			this->Remove<T>();
		}

		ResourceEntry entry;
		entry.ptr = std::make_any<T*>(resource);
		entry.deleter = [ptr = resource]() { delete ptr; };

		m_Data[id] = std::move(entry);
	}

	template<typename T>
	T* Resources::Get() {
		auto it = m_Data.find(std::type_index(typeid(T)));

		if (it != m_Data.end()) {
			return std::any_cast<T*>(it->second.ptr);
		}

		return nullptr;
	}

	template<typename T>
	void Resources::Remove() {
		std::type_index id = std::type_index(typeid(T));

		if (this->Contains<T>()) {
			ResourceEntry& p = m_Data[id];
			p.deleter();
			m_Data.erase(id);
		}
	}

	template<typename T>
	bool Resources::Contains() const noexcept {
		return m_Data.contains(std::type_index(typeid(T)));
	}

	Resources::~Resources() {
		for (auto& [id, entry] : m_Data) {
			entry.deleter();
		}

		m_Data.clear();
	}

#ifdef AXLE_TESTING
	template AXLE_TEST_API void Resources::Add<f32>(f32*);
	template AXLE_TEST_API f32* Resources::Get<f32>();
	template AXLE_TEST_API void Resources::Remove<f32>();
	template AXLE_TEST_API bool Resources::Contains<f32>() const noexcept;
	
	template AXLE_TEST_API void Resources::Add<i32>(i32*);
	template AXLE_TEST_API i32* Resources::Get<i32>();
	template AXLE_TEST_API void Resources::Remove<i32>();
	template AXLE_TEST_API bool Resources::Contains<i32>() const noexcept;
#endif // AXLE_TESTING

}
