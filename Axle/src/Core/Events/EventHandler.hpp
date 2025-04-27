#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>

#include "../Core.hpp"
#include "Event.hpp"
#include "../../Other/ObserverPattern/Observer.hpp"

//delete
#include "../Logger/Log.hpp"

namespace Axle {
	class AXLE_API EventHandler : public Subject<EventType, Event*> {
	public:
		static void Init();

		static std::shared_ptr<EventHandler>& GetInstance() {
			return m_eventHandler;
		}

		void AddEvent(Event* event);

		Subscription<EventType, Event*> Subscribe(const HandlerType& handler, EventType type);

	protected:
		void Unsubscribe(int id) override;
		void Notify(EventType type, Event* event) override;

	private:
		static std::shared_ptr<EventHandler> m_eventHandler;

		std::unordered_map<EventType, std::vector<std::shared_ptr<Event>>> m_eventsType;
		std::unordered_map<EventCategory, std::vector<std::shared_ptr<Event>>> m_eventsCategory;

		std::unordered_map<int, EventType> m_handlersType;
	};

	//test purposes (delete)
	AXLE_API void test_method(EventType type, Event* event);
}

#define AX_ADD_EVENT(...) ::Axle::EventHandler::GetInstance()->AddEvent(__VA_ARGS__)
