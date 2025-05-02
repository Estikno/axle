#pragma once

#include "axpch.hpp"

#include "../Core.hpp"

namespace Axle {
	/// Enum that defines the event type
	enum class EventType {
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	/// Enum that defines the event category
	enum class EventCategory {
		None = 0,
		Window,
		Input,
		Render
	};

	/**
	* Base class for all events.
	*
	* This class is recommended to be inherited and to add the necessary functionality of each event type.
	*/
	class AXLE_API Event {
	public:
		/**
		* Returns if the event has already been handled.
		* 
		* @returns If the event has already been handled.
		*/
		bool IsHadled() {
			return m_isHandled;
		}

		/**
		* Marks the event as handled
		*/
		virtual void Handle() {
			m_isHandled = true;
		}

		/**
		* Gets the type of event
		* 
		* @returns The type of event
		*/
		EventType GetEventType() {
			return m_eventType;
		}

		/**
		* Gets the category of the event
		* 
		* @returns The cateory of the event
		*/
		EventCategory GetEventGategory() {
			return m_eventCategory;
		}

		/**
		* Constructor
		* 
		* @param eventType The type of the event
		* @param eventCategory The category of the event
		*/
		Event(const EventType eventType, const EventCategory eventCategory);
		virtual ~Event();

	protected:
		bool m_isHandled = false;

		EventType m_eventType = EventType::None;
		EventCategory m_eventCategory = EventCategory::None;
	};
}

// Hashes the EventType enum so that it can be used with maps
namespace std {
	template <>
	struct hash<Axle::EventType> {
		size_t operator()(const Axle::EventType& eventType) const {
			// EventType is casted to int, since enums are just integer values
			return std::hash<int>()(static_cast<int>(eventType));
		}
	};
	template <>
	struct hash<Axle::EventCategory> {
		size_t operator()(const Axle::EventCategory& eventCategory) const {
			// EventCategory is casted to int, since enums are just integer values
			return std::hash<int>()(static_cast<int>(eventCategory));
		}
	};
}
