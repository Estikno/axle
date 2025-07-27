#include "axpch.hpp"

#include "Event.hpp"
#include "../Logger/Log.hpp"
#include "../Types.hpp"

namespace Axle {
	Event::Event(const EventType eventType, const EventCategory eventCategory) {
		m_eventType = eventType;
		m_eventCategory = eventCategory;
		
		AX_CORE_TRACE("Creating an event enum of type: {}", (i32)eventType);
	}
	
	Event::~Event() {
		AX_CORE_TRACE("Destroying an event enum of type: {}", (i32)m_eventType);
	}
}
