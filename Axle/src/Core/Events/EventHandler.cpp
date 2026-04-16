#include "axpch.hpp"

#include "../Types.hpp"
#include "EventHandler.hpp"
#include "../Logger/Log.hpp"
#include "Event.hpp"

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
        AX_CORE_TRACE("Added a new event of type: {}", (i32) (event.GetEventType()));

        std::scoped_lock lock(m_EventMutex);
        m_EventQueue.push_back(event);
    }

    size_t EventHandler::Subscribe(const HandlerType& handler, EventType type, EventCategory category) {
        std::scoped_lock lock(m_EventMutex);

        i32 id = m_nextId++;
        m_HandlersType[id] = std::make_pair(category, type);
        m_handlers[id] = handler;

        return id;
    }

    void EventHandler::Unsubscribe(size_t id) {
        std::scoped_lock lock(m_EventMutex);

        m_HandlersType.erase(id);
        m_handlers.erase(id);
    }

    void EventHandler::Notify(Event& event) {
        if (event.IsHandled())
            return;

        for (auto& [id, handler] : m_handlers) {
            auto it = m_HandlersType.find(id);
            if (it == m_HandlersType.end())
                continue; // Skip if the handler is not registered

            auto& [category, type] = it->second;

            if (category != event.GetEventCategory())
                continue;

            if (type == EventType::None || type == event.GetEventType())
                handler(event);
        }
    }

    void EventHandler::ProcessEvents() {
        std::vector<Event> eventsToProcess;

        // Swap the event queue with a local vector to minimize lock time and to also clear the queue for new events
        {
            std::scoped_lock lock(m_EventMutex);
            eventsToProcess.swap(m_EventQueue);
        }

        // Iterate in reverse order to ensure that the last added event is processed first
        for (auto& event : std::views::reverse(eventsToProcess)) {
            Notify(event);
        }
    }

    void EventHandler::DestroyEvents() {
        m_EventQueue.clear();
    }
} // namespace Axle
