#include "axpch.hpp"
#include "../Types.hpp"

#include "Input.hpp"
#include "../Logger/Log.hpp"
#include "../Events/EventHandler.hpp"

#include <glm/vec2.hpp>
#include "Core/Events/Event.hpp"

namespace Axle {
    InputState Input::s_InputState;

    void Input::Update() {
        s_InputState.keyboard_previous = s_InputState.keyboard_current;
        s_InputState.mouse_previous = s_InputState.mouse_current;
    }

    void Input::SetKey(Keys key, bool pressed) {
        // Only handles if the state of the key has changed
        if (s_InputState.keyboard_current.keys[static_cast<i32>(key)] != pressed) {
            // Update internal state
            s_InputState.keyboard_current.keys[static_cast<i32>(key)] = pressed;
        }

        // Fire off an event informing of the change in state
        Event event(EventType::KeyPressed, EventCategory::Input);

        if (Input::GetKeyDown(key))
            event = Event(EventType::KeyPressed, EventCategory::Input);
        else if (Input::GetKeyUp(key))
            event = Event(EventType::KeyReleased, EventCategory::Input);
        else
            event = Event(EventType::KeyIsPressed, EventCategory::Input);

        event.GetContext().u16_values[0] = static_cast<u16>(key);

        AX_ADD_EVENT(event);
    }

    void Input::SetMouseButton(MouseButtons button, bool pressed) {
        // Only handles if the state of the key has changed
        if (s_InputState.mouse_current.buttons[static_cast<i32>(button)] != pressed) {
            // Update internal state
            s_InputState.mouse_current.buttons[static_cast<i32>(button)] = pressed;
        }

        // Fire off an event informing of the change in state
        Event event(EventType::MouseButtonPressed, EventCategory::Input);

        if (Input::GetMouseButtonDown(button))
            event = Event(EventType::MouseButtonPressed, EventCategory::Input);
        else if (Input::GetMouseButtonUp(button))
            event = Event(EventType::MouseButtonReleased, EventCategory::Input);
        else
            event = Event(EventType::MouseButtonIsPressed, EventCategory::Input);

        event.GetContext().u16_values[0] = static_cast<u16>(button);

        AX_ADD_EVENT(event);
    }

    void Input::SetMousePosition(const glm::vec2& position) {
        // Only handles if the state of the key has changed
        if (s_InputState.mouse_current.position == position)
            return;

        // Update internal state
        s_InputState.mouse_current.position = position;

        // Fire off an event informing of the change in state
        Event event(EventType::MouseMoved, EventCategory::Input);

        event.GetContext().u16_values[0] = static_cast<u16>(position.x);
        event.GetContext().u16_values[1] = static_cast<u16>(position.y);

        AX_ADD_EVENT(event);
    }

    void Input::SetMouseWheel(f32 delta) {
        // Fire off an event informing of the change in state
        Event event(EventType::MouseScrolled, EventCategory::Input);

        event.GetContext().f32_values[0] = delta;

        AX_ADD_EVENT(event);
    }

    bool Input::GetKeyDown(Keys key) {
        return s_InputState.keyboard_current.keys[static_cast<i32>(key)] &&
               !s_InputState.keyboard_previous.keys[static_cast<i32>(key)];
    }

    bool Input::GetKeyUp(Keys key) {
        return !s_InputState.keyboard_current.keys[static_cast<i32>(key)] &&
               s_InputState.keyboard_previous.keys[static_cast<i32>(key)];
    }

    bool Input::GetKey(Keys key) {
        return s_InputState.keyboard_current.keys[static_cast<i32>(key)] &&
               s_InputState.keyboard_previous.keys[static_cast<i32>(key)];
    }

    bool Input::GetMouseButtonDown(MouseButtons button) {
        return s_InputState.mouse_current.buttons[static_cast<i32>(button)] &&
               !s_InputState.mouse_previous.buttons[static_cast<i32>(button)];
    }

    bool Input::GetMouseButtonUp(MouseButtons button) {
        return !s_InputState.mouse_current.buttons[static_cast<i32>(button)] &&
               s_InputState.mouse_previous.buttons[static_cast<i32>(button)];
    }

    bool Input::GetMouseButton(MouseButtons button) {
        return s_InputState.mouse_current.buttons[static_cast<i32>(button)] &&
               s_InputState.mouse_previous.buttons[static_cast<i32>(button)];
    }

    glm::vec2 Input::GetMousePosition() {
        return s_InputState.mouse_current.position;
    }

#ifdef AXLE_TESTING
    void Input::SimulateKeyState(Keys key, bool pressed) {
        SetKey(key, pressed);
    }
    void Input::SimulateMouseButtonState(MouseButtons button, bool pressed) {
        SetMouseButton(button, pressed);
    }
    void Input::SimulateMousePosition(const glm::vec2& position) {
        SetMousePosition(position);
    }
    void Input::SimulateMouseWheel(f32 delta) {
        SetMouseWheel(delta);
    }
    void Input::SimulateUpdate() {
        Update();
    }
    void Input::SimulateReset() {
        s_InputState = InputState();
    }
#endif // AXLE_TESTING

} // namespace Axle
