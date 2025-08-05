#pragma once

#include "axpch.hpp"
#include "../Types.hpp"

#include "../Core.hpp"
#include "Event.hpp"
#include "../../Other/Helpers/Observer.hpp"

namespace Axle {
	/**
	* The EventHandler class represents the event system. All events created: input, window, etc. Are handled here.
	*
	* That way everything has this class as sort of a middleman. For example, the main application class may want to receive notifications on window events, but it does not want to manually check the window class.
	*
	* This way everything is centralized, eliminating spaggeti references.
	*
	* TODO: Right now there is no buffer or multi thread safety. Every incoming event is handled and notified right away. This needs to be changed.
	*/
	class AXLE_API EventHandler : public Subject<Event*> {
	public:
		/**
		* Initializes the event handler and its singleton
		*
		* Important: This has to be called before using the macros and any other functionality
		*
		* It is safe to call multiple times, it simply displays a warning after the first call.
		*/
		static void Init();

		/**
		* Gets the event handler singleton
		*
		* @returns Returns a reference to the Event Handler
		*/
		inline static EventHandler& GetInstance() {
			return *m_EventHandler;
		}

		/**
		* Add an event to the event handler and it will be notified automatically.
		*
		* This function is not recommended to be called manually, better by the macro.
		*
		* @param event A pointer to the event added. This pointer will be entirely handled by this class, DO NOT DELETE IT YOURSELF.
		*/
		void AddEvent(Event* event);

		/**
		* Subscribes a handler (function) to the event system.
		*
		* @param handler The function that has to be called when receiving a cerating event
		* @param type The type of event it is interested in
		* @param category The category of the event
		* 
		* If you want to receive notifications of all events with in a category simply left the type as NONE
		*
		* @returns A subscription object that has to be kept on scope. As if it's not the handler will be unsubscribed automatically. This object can also be used to manage the subscription.
		*/
		Subscription<Event*> Subscribe(const HandlerType& handler, EventType type, EventCategory category);

	protected:
		/**
		* Deletes the handler type from the hashmap related to the given id.
		*
		* This method is called internally and should only be called by the "Subscription" class
		*
		* @param id Id to delete
		*/
		void Unsubscribe(i32 id) override;

		/**
		* The notify method is called internally and it notifies the suscribers about the new event that has arrived.
		*
		* Only notifies suscribers that are interested in the type of the event.
		*
		* TODO: Right now all the events stored are never removed or checked if they have been handled. This needs to be added in the future.
		*/
		void Notify(Event* event) override;

	private:
		/// The singleton of the event handler class
		static std::unique_ptr<EventHandler> m_EventHandler;
		/// A map that stores the type and category of event each handler wants (by the id)
		std::unordered_map<i32, std::pair<EventCategory, EventType>> m_HandlersType;

		/// A map that stores the events by EventType
		std::unordered_map<EventType, std::vector<std::shared_ptr<Event>>> m_EventsType;
		/// A map that stores the events by EventCategory
		std::unordered_map<EventCategory, std::vector<std::shared_ptr<Event>>> m_EventsCategory;
	};
}

/**
* Macro that simplifies the addition of new events to the event handler
*
* @param event A pointer to the event added. This pointer will be entirely handled by this class, DO NOT DELETE IT YOURSELF.
*/
#define AX_ADD_EVENT(...) ::Axle::EventHandler::GetInstance().AddEvent(__VA_ARGS__)
