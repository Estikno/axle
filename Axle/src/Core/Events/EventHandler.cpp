#include <memory>

#include "EventHandler.hpp"
#include "../Logger/Log.hpp"
#include "Event.hpp"

namespace Axle {
    std::shared_ptr<EventHandler> EventHandler::m_eventHandler;

	void EventHandler::Init() {
		if (m_eventHandler != nullptr) {
            AX_CORE_WARN("Init method of the event handler has been called a second time");
			return;
		}

		m_eventHandler = std::make_shared<EventHandler>();
	}
	
	void EventHandler::AddEvent(Event* event) {
		AX_CORE_INFO("Added a new event of type: {}", (int)(event->GetEventType()));
	}
}