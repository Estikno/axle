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
		void Add(T resource) {
			m_Data[std::type_index(typeid(T))] = std::make_any<T>(std::move(resource));
		}

		template<typename T>
		std::optional<T*> Get() {
			auto it = m_Data.find(std::type_index(typeid(T)));
			if (it != m_Data.end()) {
				return std::any_cast<T>(it->second);
			}
			return std::nullopt;
		}

		template<typename T>
		void Remove() {
			auto it = m_Data.find(std::type_index(typeid(T)));
			if (it != m_Data.end()) {
				m_Data.erase(it);
			}
		}
	private:
		std::unordered_map<std::type_index, std::any> m_Data;
	};
}
