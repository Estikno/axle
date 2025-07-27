#pragma once

#include <typeindex>
#include <typeinfo>

#include "axpch.hpp"
#include "Core/Types.hpp"

namespace Axle {
	class Resources {
	public:
		Resources() = default;

		template<typename T>
		void Add(const T* resource) = delete; // forbid const pointers

		template<typename T>
		void Add(T* resource) {
			m_Data[std::type_index(typeid(T))] = std::unique_ptr<T>(resource);
		}

		template<typename T>
		void Add(std::unique_ptr<T> resource) {
			m_Data[std::type_index(typeid(T))] = std::move(resource);
		}

		template<typename T>
		T* Get() {
			auto it = m_Data.find(std::type_index(typeid(T)));

			if (it != m_Data.end()) {
				if (auto ptr = std::any_cast<std::unique_ptr<T>>(&it->second)) {
					return ptr->get();
				}
			}

			return nullptr;
		}

		template<typename T>
		void Remove() {
			auto it = m_Data.find(std::type_index(typeid(T)));
			if (it != m_Data.end()) {
				m_Data.erase(it);
			}
		}

		template<typename T>
		bool Contains() const noexcept {
			return m_Data.find(std::type_index(typeid(T))) != m_Data.end();
		}
	private:
		std::unordered_map<std::type_index, std::any> m_Data;
	};
}
