#pragma once

#include <memory>
#include "../Core.hpp"
#include "Event.hpp"

namespace Axle {
	class AXLE_API EventHandler {
	public:
		static void Init();

		static std::shared_ptr<EventHandler>& GetInstance() {
			return m_eventHandler;
		}

		void AddEvent(Event* event);

	private:
		static std::shared_ptr<EventHandler> m_eventHandler;
	};
}
