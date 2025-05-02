#include "axpch.hpp"

#include "EventHandler.hpp"
#include "../Logger/Log.hpp"
#include "Event.hpp"
#include "../../Other/ObserverPattern/Observer.hpp"

namespace Axle {
	std::shared_ptr<EventHandler> EventHandler::m_eventHandler;

	void EventHandler::Init() {
		if (m_eventHandler != nullptr) {
			AX_CORE_WARN("Init method of the event handler has been called a second time. IGNORING");
			return;
		}

		m_eventHandler = std::make_shared<EventHandler>();

		AX_CORE_TRACE("Event handler initialized...");
	}

	void EventHandler::AddEvent(Event* event) {
		std::shared_ptr<Event> p_event(event);

		m_eventsType[event->GetEventType()].push_back(p_event);
		m_eventsCategory[event->GetEventGategory()].push_back(p_event);

		AX_CORE_INFO("Added a new event of type: {}", (int)(event->GetEventType()));

		Notify(event);
	}

	Subscription<Event*> EventHandler::Subscribe(const HandlerType& handler, EventType type, EventCategory category) {
		m_handlersType[m_nextId] = std::make_pair(category, type);
		return Subject<Event*>::Subscribe(handler);
	}

	void EventHandler::Unsubscribe(int id) {
		m_handlersType.erase(id);
		Subject<Event*>::Unsubscribe(id);
	}

	void EventHandler::Notify(Event* event) {
		for (auto& [id, handler] : m_handlers) {
			if (m_handlersType[id].first != event->GetEventGategory()) {
				continue;
			}

			if (m_handlersType[id].second == EventType::None || m_handlersType[id].second == event->GetEventType()) {
				handler(event);
			}
		}
	}
}