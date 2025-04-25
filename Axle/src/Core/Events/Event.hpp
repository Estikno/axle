#pragma once

#include <string>
#include <functional>

#include "../Core.hpp"

namespace Axle {
    enum class EventType {
        None = 0,
        WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
        AppTick, AppUpdate, AppRender,
        KeyPressed, KeyReleased,
        MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
    };

    enum EventCategory {
        None = 0,
        Window,
        Input,
        Render
    };

    class AXLE_API Event {
      public:
        bool IsHadled() {
            return m_isHandled;
        }
        virtual void Handle() {
            m_isHandled = true;
        }

        EventType GetEventType() {
            return m_eventType;
        }

        EventCategory GetEventGategory() {
            return m_eventCategory;
        }

        Event(const EventType eventType, const EventCategory eventCategory);
        virtual ~Event();

      protected:
        bool m_isHandled = false;

        EventType m_eventType = EventType::None;
        EventCategory m_eventCategory = EventCategory::None;
    };
}
