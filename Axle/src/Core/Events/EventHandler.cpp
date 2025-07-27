#include "axpch.hpp"
#include "../Types.hpp"

#include "EventHandler.hpp"
#include "../Logger/Log.hpp"
#include "Event.hpp"
#include "../../Other/ObserverPattern/Observer.hpp"

namespace Axle {
	std::unique_ptr<EventHandler> EventHandler::m_EventHandler;

	void EventHandler::Init() {
		if (m_EventHandler != nullptr) {
			AX_CORE_WARN("Init method of the event handler has been called a second time. IGNORING");
			return;
		}

		m_EventHandler = std::make_unique<EventHandler>();

		AX_CORE_TRACE("Event handler initialized...");
	}

	void EventHandler::AddEvent(Event* event) {
		std::shared_ptr<Event> p_event(event);

		m_EventsType[event->GetEventType()].push_back(p_event);
		m_EventsCategory[event->GetEventCategory()].push_back(p_event);

		AX_CORE_INFO("Added a new event of type: {}", (i32)(event->GetEventType()));

		Notify(event);
	}

	Subscription<Event*> EventHandler::Subscribe(const HandlerType& handler, EventType type, EventCategory category) {
		m_HandlersType[m_nextId] = std::make_pair(category, type);
		return Subject<Event*>::Subscribe(handler);
	}

	void EventHandler::Unsubscribe(i32 id) {
		m_HandlersType.erase(id);
		Subject<Event*>::Unsubscribe(id);
	}

	void EventHandler::Notify(Event* event) {
		for (auto& [id, handler] : m_handlers) {
			if (m_HandlersType[id].first != event->GetEventCategory()) {
				continue;
			}

			if (m_HandlersType[id].second == EventType::None || m_HandlersType[id].second == event->GetEventType()) {
				handler(event);
			}
		}
	}
}
