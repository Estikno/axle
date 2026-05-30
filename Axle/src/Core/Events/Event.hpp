#pragma once

#include "axpch.hpp"

#include "../Core.hpp"
#include "../Types.hpp"
#include "../Input/InputState.hpp"

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
        KeyTapped,
        KeySequence,
        MouseButtonPressed,
        MouseButtonIsPressed,
        MouseButtonReleased,
        MouseButtonTapped,
        MouseButtonSequence,
        MouseMoved,
        MouseScrolled,
        // Other
        AppTick,
        AppUpdate,
        AppRender,
    };

    /// Enum that defines the event category
    enum class EventCategory { None = 0, Window, Input, Render };

    /**
     * Base class for all events.
     */
    class AXLE_API Event {
    public:
        virtual ~Event() = default;

        bool IsHandled() const noexcept {
            return m_Handled;
        }
        virtual void Handle(bool handle) noexcept {
            m_Handled = handle;
        }

        virtual EventType GetEventType() const {
            return EventType::None;
        }
        virtual EventCategory GetEventCategory() const {
            return EventCategory::None;
        }

    protected:
        bool m_Handled = false;
    };

    // Macros that simplifies creating different types of events
#define DEFINE_EVENT_TYPE(type)               \
    static EventType GetStaticType() {        \
        return EventType::type;               \
    }                                         \
    EventType GetEventType() const override { \
        return GetStaticType();               \
    }

#define DEFINE_EVENT_CATEGORY(category)               \
    EventCategory GetEventCategory() const override { \
        return EventCategory::category;               \
    }

    class AXLE_API EventDispatcher {
    public:
        template <typename T>
        using EventFn = std::function<bool(T&)>;

        EventDispatcher(Event& event)
            : m_Event(event) {}

        /**
         * Dispatches the event to the given function if it coincides the type
         *
         * @returns true if the event was dispatched succesfully, false otherwise
         * */
        template <typename T>
        bool Dispatch(EventFn<T> func) {
            if (m_Event.GetEventType() == T::GetStaticType()) {
                m_Event.Handle(func(static_cast<T&>(m_Event)));
                return true;
            }
            return false;
        }

    private:
        Event& m_Event;
    };

    // Different event types
    // ---------------------
    class WindowCloseEvent : public Event {
    public:
        WindowCloseEvent() {}

        DEFINE_EVENT_TYPE(WindowClose);
        DEFINE_EVENT_CATEGORY(Window);
    };

    class WindowResizeEvent : public Event {
    public:
        WindowResizeEvent(u32 width, u32 height)
            : m_Width(width),
              m_Height(height) {}

        u32 GetWidth() const noexcept {
            return m_Width;
        }
        u32 GetHeight() const noexcept {
            return m_Height;
        }

        DEFINE_EVENT_TYPE(WindowResize);
        DEFINE_EVENT_CATEGORY(Window);

    private:
        u32 m_Width;
        u32 m_Height;
    };

    class KeyEvent : public Event {
    public:
        Keys GetKey() const noexcept {
            return m_Key;
        }

        DEFINE_EVENT_CATEGORY(Input);

    protected:
        KeyEvent(Keys key)
            : m_Key(key) {}

    private:
        Keys m_Key;
    };

    class KeyPressedEvent : public KeyEvent {
    public:
        KeyPressedEvent(Keys key)
            : KeyEvent(key) {}

        DEFINE_EVENT_TYPE(KeyPressed);
    };

    class KeyIsPressedEvent : public KeyEvent {
    public:
        KeyIsPressedEvent(Keys key)
            : KeyEvent(key) {}

        DEFINE_EVENT_TYPE(KeyIsPressed);
    };

    class KeyReleasedEvent : public KeyEvent {
    public:
        KeyReleasedEvent(Keys key)
            : KeyEvent(key) {}

        DEFINE_EVENT_TYPE(KeyReleased);
    };

    class KeyTappedEvent : public KeyEvent {
    public:
        KeyTappedEvent(Keys key, u8 count)
            : KeyEvent(key),
              m_Count(count) {}

        u8 GetCount() const noexcept {
            return m_Count;
        }

        DEFINE_EVENT_TYPE(KeyTapped);

    private:
        u8 m_Count;
    };

    class KeySequenceEvent : public Event {
    public:
        KeySequenceEvent(u32 id)
            : m_ID(id) {}

        u32 GetID() const noexcept {
            return m_ID;
        }

        DEFINE_EVENT_TYPE(KeySequence);
        DEFINE_EVENT_CATEGORY(Input);

    private:
        u32 m_ID;
    };

    class MouseButtonEvent : public Event {
    public:
        MouseButtons GetMouseButton() const noexcept {
            return m_Button;
        }

        DEFINE_EVENT_CATEGORY(Input);

    protected:
        MouseButtonEvent(MouseButtons button)
            : m_Button(button) {}

    private:
        MouseButtons m_Button;
    };

    class MouseButtonPressedEvent : public MouseButtonEvent {
    public:
        MouseButtonPressedEvent(MouseButtons button)
            : MouseButtonEvent(button) {}

        DEFINE_EVENT_TYPE(MouseButtonPressed);
    };

    class MouseButtonIsPressedEvent : public MouseButtonEvent {
    public:
        MouseButtonIsPressedEvent(MouseButtons button)
            : MouseButtonEvent(button) {}

        DEFINE_EVENT_TYPE(MouseButtonIsPressed);
    };

    class MouseButtonReleasedEvent : public MouseButtonEvent {
    public:
        MouseButtonReleasedEvent(MouseButtons button)
            : MouseButtonEvent(button) {}

        DEFINE_EVENT_TYPE(MouseButtonReleased);
    };

    class MouseButtonTappedEvent : public MouseButtonEvent {
    public:
        MouseButtonTappedEvent(MouseButtons button, u8 count)
            : MouseButtonEvent(button),
              m_Count(count) {}

        u8 GetCount() const noexcept {
            return m_Count;
        }

        DEFINE_EVENT_TYPE(MouseButtonTapped);

    private:
        u8 m_Count;
    };

    class MouseButtonSequenceEvent : public Event {
    public:
        MouseButtonSequenceEvent(u32 id)
            : m_ID(id) {}

        u32 GetID() const noexcept {
            return m_ID;
        }

        DEFINE_EVENT_TYPE(MouseButtonSequence);
        DEFINE_EVENT_CATEGORY(Input);

    private:
        u32 m_ID;
    };

    class MouseMovedEvent : public Event {
    public:
        MouseMovedEvent(f32 x, f32 y)
            : m_X(x),
              m_Y(y) {}

        f32 GetX() const noexcept {
            return m_X;
        }
        f32 GetY() const noexcept {
            return m_Y;
        }

        DEFINE_EVENT_TYPE(MouseMoved);
        DEFINE_EVENT_CATEGORY(Input);

    private:
        f32 m_X;
        f32 m_Y;
    };

    class MouseScrollEvent : public Event {
    public:
        MouseScrollEvent(f32 xoffset, f32 yoffset)
            : m_xOffset(xoffset),
              m_yOffset(yoffset) {}

        f32 GetXOffset() const noexcept {
            return m_xOffset;
        }
        f32 GetYOffset() const noexcept {
            return m_yOffset;
        }

        DEFINE_EVENT_TYPE(MouseScrolled);
        DEFINE_EVENT_CATEGORY(Input);

    private:
        f32 m_xOffset, m_yOffset;
    };
    // ---------------------
} // namespace Axle
