#include "axpch.hpp"

#include "../Types.hpp"
#include "EventHandler.hpp"
#include "../Logger/Log.hpp"
#include "Event.hpp"
#include <tuple>

namespace Axle {
    std::unique_ptr<EventHandler> EventHandler::m_EventHandler;

    void EventHandler::Init() {
        if (m_EventHandler != nullptr) {
            AX_CORE_WARN("Init method of the event handler has been called a second time. IGNORING");
            return;
        }

        m_EventHandler = std::make_unique<EventHandler>();

        AX_CORE_INFO("Event handler initialized...");
    }

    void EventHandler::ShutDown() {
        m_EventHandler.reset();
        AX_CORE_INFO("Event handler deleted...");
    }

    void EventHandler::AddEvent(Event event) {
        std::scoped_lock lock(m_EventMutex);

        AX_CORE_TRACE("Added a new event of type: {}", (i32) (event.GetEventType()));
        m_EventQueue.push_back(event);
    }

    size_t EventHandler::Subscribe(const HandlerType& handler, EventType type, EventCategory category) {
        std::scoped_lock lock(m_HandlersMutex);

        size_t id = m_nextId++;
        m_handlers[id] = std::make_tuple(handler, category, type);

        return id;
    }

    void EventHandler::Unsubscribe(size_t id) {
        std::scoped_lock lock(m_HandlersMutex);

        m_handlers.erase(id);
    }

    void EventHandler::Notify(Event& event) {
        if (event.IsHandled())
            return;

        std::vector<HandlerType> snapshot;

        {
            std::scoped_lock lock(m_HandlersMutex);
            for (auto& [id, tuple] : m_handlers) {
                auto& [handler, category, type] = tuple;
                if (category == event.GetEventCategory() && (type == EventType::None || type == event.GetEventType()))
                    snapshot.push_back(handler);
            }
        }

        for (auto& handler : snapshot) {
            if (event.IsHandled())
                return;
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

        for (auto& event : eventsToProcess) {
            Notify(event);
        }
    }
} // namespace Axle
