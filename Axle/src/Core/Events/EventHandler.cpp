#include "axpch.hpp"

#include "Core/Application.hpp"
#include "../Types.hpp"
#include "EventHandler.hpp"
#include "../Logger/Log.hpp"
#include "Event.hpp"

namespace Axle {
    std::unique_ptr<EventHandler> EventHandler::m_EventHandler;

    void EventHandler::Init() {
        if (m_EventHandler != nullptr) {
            AX_CORE_WARN(LogChannel::Events,
                         "Init method of the event handler has been called a second time. IGNORING");
            return;
        }

        m_EventHandler = std::make_unique<EventHandler>();

        AX_CORE_INFO(LogChannel::Events, "Event handler initialized...");
    }

    void EventHandler::ShutDown() {
        m_EventHandler.reset();
        AX_CORE_INFO(LogChannel::Events, "Event handler deleted...");
    }

    void EventHandler::SubmitEvent(std::unique_ptr<Event> event) {
        std::scoped_lock lock(m_Mutex);
        m_EventQueue.emplace_back(std::move(event));
    }

    void EventHandler::Notify(Event& event,
                              std::vector<Layer*>::reverse_iterator begin,
                              std::vector<Layer*>::reverse_iterator end) {
        // Always send to the Application first
        Application::GetInstance().OnEvent(event);
        if (event.IsHandled())
            return;

        for (; begin != end; begin++) {
            if (event.IsHandled())
                break;
            (*begin)->OnEvent(event);
        }
    }

    void EventHandler::ProcessEvents(std::vector<Layer*>::reverse_iterator begin,
                                     std::vector<Layer*>::reverse_iterator end) {
        std::vector<std::unique_ptr<Event>> eventsToProcess;

        // Swap the event queue with a local vector to minimize lock time and to also clear the queue for new events
        {
            std::scoped_lock lock(m_Mutex);
            eventsToProcess.swap(m_EventQueue);
        }

        for (auto& event : eventsToProcess) {
            Notify(*event, begin, end);
        }
    }
} // namespace Axle
