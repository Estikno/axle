#pragma once

#include "axpch.hpp"

#include "../Core.hpp"
#include "../Types.hpp"

namespace Axle {
    /// Enum that defines the event type
    enum class EventType {
        None = 0,
        // Window
        WindowClose,
        WindowResize,
        WindowFocus,
        WindowLostFocus,
        WindowMoved,
        // Input
        KeyPressed,
        KeyIsPressed,
        KeyReleased,
        MouseButtonPressed,
        MouseButtonIsPressed,
        MouseButtonReleased,
        MouseMoved,
        MouseScrolled,
        // Other
        AppTick,
        AppUpdate,
        AppRender,
    };

    /// Enum that defines the event category
    enum class EventCategory { None = 0, Window, Input, Render };

    enum class SubscribeLayer { Debug = 0, UI, Render, Other };

    /**
     * @brief Context of the event. Contains data that can be used by the event.
     *
     * Multiple data can be used.
     */
    struct EventContext {
        /// Optional custom data that can be used by the event.
        /// The pointer retrieved shall be deleted by the retriever.
        std::optional<std::any> custom_data;

        // Raw numeric / fixed-size data (128 bits in total)
        union {
            /// 2 x 64-bit signed integers
            i64 i64_values[2];

            /// 2 x 64-bit unsigned integers
            u64 u64_values[2];

            /// 2 x 64-bit floating-point numbers
            f64 f64_values[2];

            /// 4 x 32-bit signed integers
            i32 i32_values[4];

            /// 4 x 32-bit unsigned integers
            u32 u32_values[4];

            /// 4 x 32-bit floats
            f32 f32_values[4];

            /// 8 x 16-bit signed integers
            i16 i16_values[8];

            /// 8 x 16-bit unsigned integers
            u16 u16_values[8];

            /// 16 x 8-bit signed integers
            i8 i8_values[16];

            /// 16 x 8-bit unsigned integers
            u8 u8_values[16];
        };

        EventContext()
            : custom_data(std::nullopt),
              u64_values{0, 0} {}
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
            return m_Handled;
        }

        /**
         * Marks the event as handled
         */
        virtual void Handle() {
            m_Handled = true;
        }

        /**
         * Gets the type of event
         *
         * @returns The type of event
         */
        EventType GetEventType() {
            return m_EventType;
        }

        /**
         * Gets the category of the event
         *
         * @returns The cateory of the event
         */
        EventCategory GetEventCategory() {
            return m_EventCategory;
        }

        EventContext& GetContext() {
            return m_Context;
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
        bool m_Handled = false;

        EventType m_EventType = EventType::None;
        EventCategory m_EventCategory = EventCategory::None;

        EventContext m_Context = {};
    };
} // namespace Axle

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
} // namespace std
