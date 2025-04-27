#pragma once

#include <vector>
#include <functional>
#include <unordered_map>

#include "../../Core/Events/Event.hpp"
#include "../../Core/Core.hpp"

namespace Axle {
	// Forward declaration
	template<typename... Args>
	class AXLE_API Subject;

	// Represents an active subscription to a Subject
	template<typename... Args>
	class AXLE_API Subscription {
	public:
		Subscription(Subject<Args...>* subject, int id)
			: m_subject(subject), m_id(id) { }
		~Subscription() {
			Unsubscribe();
		}

		Subscription(Subscription&& other) noexcept
			: m_subject(other.m_subject), m_id(other.m_id), m_unsubscribed(other.m_unsubscribed) {
			other.m_subject = nullptr;
		}
		Subscription& operator=(Subscription&& other) noexcept {
			if (this != &other) {
				Unsubscribe();

				m_subject = other.m_subject;
				m_id = other.m_id;
				m_unsubscribed = other.m_unsubscribed;

				other.m_subject = nullptr;
			}
		}

		Subscription(const Subscription&) = delete;
		Subscription& operator=(const Subscription&) = delete;

		void Unsubscribe() {
			if (!m_unsubscribed && m_subject) {
				m_subject->Unsubscribe(m_id);
				m_unsubscribed = true;
			}
		}

	private:
		Subject<Args...>* m_subject;
		int m_id;
		bool m_unsubscribed = false;
	};

	// Represents the source of events or notifications
	template<typename... Args>
	class AXLE_API Subject {
	public:
		using HandlerType = std::function<void(Args...)>;

		virtual Subscription<Args...> Subscribe(const HandlerType& handler) {
			int id = m_nextId++;
			m_handlers[id] = handler;
			return Subscription<Args...>(this, id);
		}

		virtual ~Subject() = default;
		virtual void Notify(Args... args) {
			for (auto& [id, handler] : m_handlers) {
				handler(args...);
			}
		}

	protected:
		friend class Subscription<Args...>;

		virtual void Unsubscribe(int id) {
			m_handlers.erase(id);
		}

		std::unordered_map<int, HandlerType> m_handlers;
		int m_nextId = 0;
	};
}
