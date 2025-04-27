#include "Observer.hpp"

namespace Axle {
	// Explicit template instantiations for Subscription and Subject with EventType and Event*
	template class Subscription<EventType, Event*>;
	template class Subject<EventType, Event*>;

	template<typename... Args>
	Subscription<Args...>::Subscription(Subject<Args...>* subject, int id)
		: m_subject(subject), m_id(id) { }

	template<typename... Args>
	Subscription<Args...>::~Subscription() {
		Unsubscribe(); // Safe: checks if already unsubscribed
	}

	template<typename... Args>
	Subscription<Args...>::Subscription(Subscription&& other) noexcept
		: m_subject(other.m_subject), m_id(other.m_id), m_unsubscribed(other.m_unsubscribed) {
		other.m_subject = nullptr;
	}

	template<typename... Args>
	Subscription<Args...>& Subscription<Args...>::operator=(Subscription&& other) noexcept {
		if (this != &other) {
			Unsubscribe(); // Unsubscribed current first (if needed)
			
			m_subject = other.m_subject;
			m_id = other.m_id;
			m_unsubscribed = other.m_unsubscribed;

			other.m_subject = nullptr;
		}

		return *this;
	}

	template<typename... Args>
	void Subscription<Args...>::Unsubscribe() {
		if (!m_unsubscribed && m_subject) {
			m_subject->Unsubscribe(m_id);
			m_unsubscribed = true;
		}
	}

	// Subject implementation
	template<typename... Args>
	Subscription<Args...> Subject<Args...>::Subscribe(const HandlerType& handler) {
		int id = m_nextId++;
		m_handlers[id] = handler;
		return Subscription<Args...>(this, id);
	}

	template<typename... Args>
	void Subject<Args...>::Notify(Args... args) {
		for (auto& [id, handler] : m_handlers) {
			handler(args...);
		}
	}

	template<typename... Args>
	void Subject<Args...>::Unsubscribe(int id) {
		m_handlers.erase(id);
	}
}
