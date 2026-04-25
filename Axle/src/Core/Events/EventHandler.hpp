#pragma once

#include "axpch.hpp"

#include "../Core.hpp"
#include "Event.hpp"
#include <cstddef>
#include <tuple>

namespace Axle {
    /**
     * The EventHandler class represents the event system. All events created: input, window, etc. Are handled here.
     *
     * That way everything has this class as sort of a middleman. For example, the main application class may want to
     * receive notifications on window events, but it does not want to manually check the window class.
     *
     * This way everything is centralized, eliminating spaggeti references.
     */
    class AXLE_API EventHandler {
    public:
        /// The type of function the subject accepts
        using HandlerType = std::function<void(Event&)>;

        EventHandler(const EventHandler&) = delete;
        EventHandler& operator=(const EventHandler&) = delete;

        EventHandler() {}
        ~EventHandler() {}

        /**
         * Initializes the event handler and its singleton
         *
         * Important: This has to be called before using the macros and any other functionality
         * This function is NOT thread safe so it must be called from only one thread and only once to be safe.
         *
         * It is safe to call multiple times, it simply displays a warning after the first call.
         */
        static void Init();

        /**
         * Shutdowns the manager, important to call when no other component depends on it anymore
         * This function is NOT thread safe so it must be called from only one thread and only once to be safe.
         */
        static void ShutDown();

        /**
         * Gets the event handler singleton
         *
         * The manager has to have already been initilized before getting the instance.
         *
         * @returns Returns a reference to the Event Handler
         */
        inline static EventHandler& GetInstance() {
            return *m_EventHandler;
        }

        // TODO: Optimize how to add events as now they are coppied
        /**
         * Add an event to the event handler and it will be notified automatically.
         * This function is not recommended to be called manually, better by the macro.
         *
         * @param event An event object
         */
        void AddEvent(Event event);

        /**
         * Subscribes a handler (function) to the event system.
         *
         * Note that the order is first in last out. The last subscribed handler will be the first to be called.
         *
         * @param handler The function that has to be called when receiving a cerating event
         * @param type The type of event it is interested in
         * @param category The category of the event
         *
         * If you want to receive notifications of all events with in a category simply left the type as NONE
         *
         * @returns A a subscription id that can be used to unsubscribe later
         */
        size_t Subscribe(const HandlerType& handler, EventType type, EventCategory category);

        /**
         * Processes all the events in the queue and notifies the subscribers.
         *
         * This method should be called every frame to ensure that all events are processed.
         *
         * It is safe to call this method multiple times per frame, but it is recommended to call it only once.
         */
        void ProcessEvents();

        /**
         * Deletes the handler type from the hashmap related to the given id.
         *
         * This method is called internally and should only be called by the "Subscription" class
         *
         * @param id Id to delete
         */
        void Unsubscribe(size_t id);

    protected:
        /**
         * The notify method is called internally and it notifies the suscribers about the new event that has arrived.
         *
         * Only notifies suscribers that are interested in the type of the event.
         */
        void Notify(Event& event);

    private:
        /// The singleton of the event handler class
        static std::unique_ptr<EventHandler> m_EventHandler;

        std::vector<Event> m_EventQueue;

        std::mutex m_EventMutex;
        std::mutex m_HandlersMutex;

        /// Active handlers mapped by their ID
        std::unordered_map<size_t, std::tuple<HandlerType, EventCategory, EventType>> m_handlers;
        /// Next unique ID for new subscriptions
        size_t m_nextId = 0;
    };
} // namespace Axle

/**
 * Macro that simplifies the addition of new events to the event handler
 */
#define AX_ADD_EVENT(...) ::Axle::EventHandler::GetInstance().AddEvent(__VA_ARGS__)
