#pragma once

#include "axpch.hpp"

#include "../Core.hpp"

namespace Axle {
	/// Enum that defines the event type
	enum class EventType {
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		//Input
		KeyPressed, KeyIsPressed, KeyReleased,
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
	* @brief Context of the event. Contains data that can be used by the event.
	* 
	* Multiple data can be used.
	*/
	struct EventContext {
		// 128 bits
		union {
			/// 2 x 64-bit signed integers
			long long i64[2];

			/// 2 x 64-bit unsigned integers
			unsigned long long u64[2];

			/// 2 x 64-bit floating-point numbers
			double f64[2];

			/// 4 x 32-bit signed integers
			int i32[4];

			/// 4 x 32-bit unsigned integers
			unsigned int u32[4];

			/// 4 x 32-bit floats
			float f32[4];

			/// 8 x 16-bit signed integers
			short i16[8];

			/// 8 x 16-bit unsigned integers
			unsigned short u16[8];

			/// 16 x 8-bit signed integers
			signed char i8[16];

			/// 16 x 8-bit unsigned integers
			unsigned char u8[16];

			/**
			* @brief Allows a pointer to arbitrary data to be passed. Also includes size info.
			*
			* NOTE: If used, should be freed by the sender or listener. Normally by the listener.
			*/
			struct {
				// The size of the data pointed to (in bytes)
				unsigned long long size;
				// A pointer to a memory block of data to be included with the event.
				void* data;
			} custom_data;
		};
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

		EventContext& GetContext() {
			return m_context;
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

		EventContext m_context = {};
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
