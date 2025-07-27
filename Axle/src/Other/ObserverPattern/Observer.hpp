#pragma once

#include "axpch.hpp"
#include "Core/Types.hpp"

namespace Axle {
	// Forward declaration
	template<typename... Args>
	class Subject;

	/**
	* Represents an active subscription to a Subject.
	*
	* It automatically unsubscribes when destroyed, unless already manually unsubscribed.
	*/
	template<typename... Args>
	class Subscription {
	public:
		/**
		* Constructs a Subscription associated with a Subject and a handler ID
		*
		* @param subject The subject it would be associated to
		* @param id A simple identifier
		*/
		Subscription(Subject<Args...>* subject, i32 id)
			: m_subject(subject), m_id(id) {
		}

		// Destructor automatically unsubscribes if not already done
		~Subscription() {
			Unsubscribe();
		}

		// Move constructor: transfers ownership from another subscription
		Subscription(Subscription&& other) noexcept
			: m_subject(other.m_subject), m_id(other.m_id), m_unsubscribed(other.m_unsubscribed) {
			other.m_subject = nullptr;
		}

		// Move assignment: safely unsubscribes current, then moves from another subscription
		Subscription& operator=(Subscription&& other) noexcept {
			if (this != &other) {
				Unsubscribe();

				m_subject = other.m_subject;
				m_id = other.m_id;
				m_unsubscribed = other.m_unsubscribed;

				other.m_subject = nullptr;
			}
		}

		// No copy allowed (ensures unique ownership of the subscription)
		Subscription(const Subscription&) = delete;
		Subscription& operator=(const Subscription&) = delete;

		/**
		* Manually unsubscribe from the Subject.
		*
		* Safe to call multiple times.
		*/
		void Unsubscribe() {
			if (!m_unsubscribed && m_subject) {
				m_subject->Unsubscribe(m_id);
				m_unsubscribed = true;
			}
		}

	private:
		/// Pointer back to the Subject
		Subject<Args...>* m_subject;
		/// Unique ID of the subscription
		i32 m_id;
		/// Whether already unsubscribed
		bool m_unsubscribed = false;
	};

	/**
	* Represents a source of events or notifications
	* Observers can subscribe handlers (functions) to this subject.
	*/
	template<typename... Args>
	class Subject {
	public:
		/// The type of function the subject accepts
		using HandlerType = std::function<void(Args...)>;

		/**
		* Subscribe a new handler to this Subject.
		*
		* @param handler A callable that accepts (Args...) arguments.
		* @return Subscription object that manages the subscription lifetime.
		*/
		virtual Subscription<Args...> Subscribe(const HandlerType& handler) {
			i32 id = m_nextId++;
			m_handlers[id] = handler;
			return Subscription<Args...>(this, id);
		}

		virtual ~Subject() = default;

		/**
		* Notify all subscribed handlers with provided arguments
		*/
		virtual void Notify(Args... args) {
			for (auto& [id, handler] : m_handlers) {
				handler(args...);
			}
		}

	protected:
		// Allow the Subscription class to access protected values in order to call the Unsubscribe method
		friend class Subscription<Args...>;

		/**
		* Remove a subscription by its ID
		*
		* Called internally by Subscription when unsubscribing
		*/
		virtual void Unsubscribe(i32 id) {
			m_handlers.erase(id);
		}

		/// Active handlers mapped by their ID
		std::unordered_map<i32, HandlerType> m_handlers;
		/// Next unique ID for new subscriptions
		i32 m_nextId = 0;
	};
}
