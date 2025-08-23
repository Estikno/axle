#include "axpch.hpp"
#include "../Types.hpp"

#include "EventHandler.hpp"
#include "../Logger/Log.hpp"
#include "Event.hpp"
#include "../../Other/Helpers/Observer.hpp"

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

    void EventHandler::AddEvent(Event event) {
        AX_CORE_INFO("Added a new event of type: {}", (i32) (event.GetEventType()));
        m_EventQueue.push_back(std::move(event));
    }

    Subscription<Event> EventHandler::Subscribe(const HandlerType& handler, EventType type, EventCategory category) {
        m_HandlersType[m_nextId] = std::make_pair(category, type);
        return Subject<Event>::Subscribe(handler);
    }

    void EventHandler::Unsubscribe(size_t id) {
        m_HandlersType.erase(id);
        Subject<Event>::Unsubscribe(id);
    }

    void EventHandler::Notify(Event* event) {
        for (auto& [id, handler] : m_handlers) {
            auto it = m_HandlersType.find(id);
            if (it == m_HandlersType.end())
                continue; // Skip if the handler is not registered

            auto& [category, type] = it->second;

            if (category != event->GetEventCategory())
                continue;

            if (type == EventType::None || type == event->GetEventType())
                handler(event);
        }
    }

    void EventHandler::ProcessEvents() {
        for (auto& event : m_EventQueue) {
            Notify(&event);
        }

        m_EventQueue.clear();
    }

    void EventHandler::DestroyEvents() {
        m_EventQueue.clear();
    }
} // namespace Axle
