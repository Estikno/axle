#include "axpch.hpp"

#include "Core/Core.hpp"
#include "Core/Types.hpp"
#include "Core/Logger/Log.hpp"

#include "Resources.hpp"

namespace Axle {
	template<typename T>
	void Resources::Add(T* resource) {
		Add<T>(std::shared_ptr<T>(resource));
	}

	template<typename T>
	void Resources::Add(std::shared_ptr<T> resource) {
		std::type_index id = std::type_index(typeid(T));

		if (resource == nullptr) {
			AX_CORE_ERROR("Cannot add a null resource to the resource manager.");
			return;
		}

		if (Contains<T>()) {
			AX_CORE_WARN("Overwriting resource of type: {0} because it already exists in the resource manager.", typeid(T).name());
			m_Data.erase(id);
		}

		m_Data[id] = std::static_pointer_cast<void>(resource);
	}

	template<typename T>
	T* Resources::Get() {
		auto it = m_Data.find(std::type_index(typeid(T)));

		if (it != m_Data.end()) {
			return static_cast<T*>(it->second.get());
		}

		return nullptr;
	}

	template<typename T>
	std::shared_ptr<T> Resources::GetShared() {
		auto it = m_Data.find(std::type_index(typeid(T)));

		if (it != m_Data.end()) {
			return std::static_pointer_cast<T>(it->second);
		}

		return nullptr;
	}

	template<typename T>
	void Resources::Remove() {
		m_Data.erase(std::type_index(typeid(T)));
	}

	template<typename T>
	bool Resources::Contains() const noexcept {
		return m_Data.contains(std::type_index(typeid(T)));
	}

#ifdef AXLE_TESTING
	template AXLE_TEST_API void Resources::Add<f32>(f32*);
	template AXLE_TEST_API void Resources::Add<f32>(std::shared_ptr<f32>);
	template AXLE_TEST_API f32* Resources::Get<f32>();
	template AXLE_TEST_API std::shared_ptr<f32> Resources::GetShared<f32>();
	template AXLE_TEST_API void Resources::Remove<f32>();
	template AXLE_TEST_API bool Resources::Contains<f32>() const noexcept;
	
	template AXLE_TEST_API void Resources::Add<i32>(i32*);
	template AXLE_TEST_API void Resources::Add<i32>(std::shared_ptr<i32>);
	template AXLE_TEST_API i32* Resources::Get<i32>();
	template AXLE_TEST_API std::shared_ptr<i32> Resources::GetShared<i32>();
	template AXLE_TEST_API void Resources::Remove<i32>();
	template AXLE_TEST_API bool Resources::Contains<i32>() const noexcept;
#endif // AXLE_TESTING
}
