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
		Subscription(Subject<Args...>* subject, int id);
		~Subscription();

		Subscription(Subscription&& other) noexcept;
		Subscription& operator=(Subscription&& other) noexcept;

		Subscription(const Subscription&) = delete;
		Subscription& operator=(const Subscription&) = delete;

		void Unsubscribe();

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

		virtual Subscription<Args...> Subscribe(const HandlerType& handler);

		virtual ~Subject() = default;
		virtual void Notify(Args... args);

	protected:
		friend class Subscription<Args...>;

		virtual void Unsubscribe(int id);

		std::unordered_map<int, HandlerType> m_handlers;
		int m_nextId = 0;
	};
}
