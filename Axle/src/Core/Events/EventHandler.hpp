#pragma once

#include "axpch.hpp"

#include "../Core.hpp"
#include "Event.hpp"
#include "Core/Layer/Layer.hpp"

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
        inline static EventHandler& GetInstance() noexcept {
            return *m_EventHandler;
        }

        /**
         * Add an event to the event handler and it will be notified automatically.
         * This function is not recommended to be called manually, better by the macro.
         *
         * @param event An event lvalue object
         */
        void DispatchEvent(const Event& event);

        /**
         * Add an event to the event handler and it will be notified automatically.
         * This function is not recommended to be called manually, better by the macro.
         *
         * @param event An event rvalue object
         */
        void DispatchEvent(Event&& event);

        /**
         * Processes all the events in the queue and notifies the subscribers.
         * This method should be called every frame to ensure that all events are processed.
         *
         * It is safe to call this method multiple times per frame, but it is recommended to call it only once.
         *
         * @param begin The begining iterator of the layer you want to push events to
         * @param end The end iterator of the layers
         */
        void ProcessEvents(std::vector<Layer*>::reverse_iterator begin, std::vector<Layer*>::reverse_iterator end);

    private:
        /**
         * The notify method is called internally and it notifies the suscribers about the new event that has arrived.
         *
         * @param begin The begining iterator of the layer you want to push events to
         * @param end The end iterator of the layers
         */
        void
        Notify(Event& event, std::vector<Layer*>::reverse_iterator begin, std::vector<Layer*>::reverse_iterator end);

        /// The singleton of the event handler class
        static std::unique_ptr<EventHandler> m_EventHandler;

        std::vector<Event> m_EventQueue;

        std::mutex m_EventMutex;
    };
} // namespace Axle

/**
 * Macro that simplifies the addition of new events to the event handler
 */
#define AX_DISPATCH_EVENT(...) ::Axle::EventHandler::GetInstance().DispatchEvent(__VA_ARGS__)
