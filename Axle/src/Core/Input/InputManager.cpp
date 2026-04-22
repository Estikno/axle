#include "axpch.hpp"
#include "../Types.hpp"

#include "InputManager.hpp"
#include "../Logger/Log.hpp"
#include "../Events/EventHandler.hpp"
#include "Core/Events/Event.hpp"
#include "Core/Input/InputState.hpp"

#include <array>
#include <glm/vec2.hpp>

namespace Axle {
    std::unique_ptr<InputManager> InputManager::m_InputManager;

    void InputManager::Init() {
        if (m_InputManager != nullptr) {
            AX_CORE_WARN("Init method of the Input Manager has been called a second time. IGNORING");
            return;
        }

        m_InputManager = std::make_unique<InputManager>();

        AX_CORE_INFO("Input Manager initialized...");
    }

    void InputManager::ShutDown() {
        m_InputManager.reset();
        AX_CORE_INFO("Input Manager deleted...");
    }

    void InputManager::Update() {
        // Send events if keys are pressed
        for (u16 i = 0; i < static_cast<u16>(Keys::MaxKeys); ++i) {
            if (GetKey(static_cast<Keys>(i))) {
                Event event(EventType::KeyIsPressed, EventCategory::Input);
                event.GetContext().raw_data = std::array<u16, 8>{i};
                AX_ADD_EVENT(event);
            }
        }

        // Same for mouse buttons
        for (u16 i = 0; i < static_cast<u16>(MouseButtons::MAX_BUTTONS); ++i) {
            if (GetMouseButton(static_cast<MouseButtons>(i))) {
                Event event(EventType::MouseButtonIsPressed, EventCategory::Input);
                event.GetContext().raw_data = std::array<u16, 8>{i};
                AX_ADD_EVENT(event);
            }
        }

        std::scoped_lock lock(m_InputMutex);
        m_InputState.keyboard_previous = m_InputState.keyboard_current;
        m_InputState.mouse_previous = m_InputState.mouse_current;
    }

    void InputManager::SetKey(Keys key, bool pressed) {
        {
            std::scoped_lock lock(m_InputMutex);
            // Only handles if the state of the key has changed
            if (m_InputState.keyboard_current.keys[static_cast<i32>(key)] != pressed) {
                // Update internal state
                m_InputState.keyboard_current.keys[static_cast<i32>(key)] = pressed;
            }
        }

        // Fire off an event informing of the change in state
        Event event(EventType::KeyIsPressed, EventCategory::Input);

        if (GetKeyDown(key))
            event = Event(EventType::KeyPressed, EventCategory::Input);
        else if (GetKeyUp(key))
            event = Event(EventType::KeyReleased, EventCategory::Input);
        else
            event = Event(EventType::KeyIsPressed, EventCategory::Input);

        // event->GetContext().u16_values[0] = static_cast<u16>(key);
        event.GetContext().raw_data = std::array<u16, 8>{static_cast<u16>(key)};

        AX_ADD_EVENT(event);
    }

    void InputManager::SetMouseButton(MouseButtons button, bool pressed) {
        {
            std::scoped_lock lock(m_InputMutex);
            // Only handles if the state of the key has changed
            if (m_InputState.mouse_current.buttons[static_cast<i32>(button)] != pressed) {
                // Update internal state
                m_InputState.mouse_current.buttons[static_cast<i32>(button)] = pressed;
            }
        }

        // Fire off an event informing of the change in state
        Event event(EventType::MouseButtonIsPressed, EventCategory::Input);

        if (GetMouseButtonDown(button))
            event = Event(EventType::MouseButtonPressed, EventCategory::Input);
        else if (GetMouseButtonUp(button))
            event = Event(EventType::MouseButtonReleased, EventCategory::Input);
        else
            event = Event(EventType::MouseButtonIsPressed, EventCategory::Input);

        // event->GetContext().u16_values[0] = static_cast<u16>(button);
        event.GetContext().raw_data = std::array<u16, 8>{static_cast<u16>(button)};

        AX_ADD_EVENT(event);
    }

    void InputManager::SetMousePosition(const glm::vec2& position) {
        bool stateChange = false;

        {
            std::scoped_lock lock(m_InputMutex);
            // Only handles if the state of the key has changed
            if (m_InputState.mouse_current.position != position) {
                // Update internal state
                m_InputState.mouse_current.position = position;
                stateChange = true;
            }
        }

        if (!stateChange)
            return;

        // Fire off an event informing of the change in state
        Event event(EventType::MouseMoved, EventCategory::Input);

        // event->GetContext().u16_values[0] = static_cast<u16>(position.x);
        // event->GetContext().u16_values[1] = static_cast<u16>(position.y);
        event.GetContext().raw_data = std::array<u16, 8>{static_cast<u16>(position.x), static_cast<u16>(position.y)};

        AX_ADD_EVENT(event);
    }

    void InputManager::SetMouseWheel(f32 delta) {
        // Fire off an event informing of the change in state
        Event event(EventType::MouseScrolled, EventCategory::Input);
        // event->GetContext().f32_values[0] = delta;
        event.GetContext().raw_data = std::array<f32, 4>{static_cast<f32>(delta)};

        AX_ADD_EVENT(event);
    }

    bool InputManager::GetKeyDown(Keys key) {
        std::scoped_lock lock(m_InputMutex);
        return m_InputState.keyboard_current.keys[static_cast<i32>(key)] &&
               !m_InputState.keyboard_previous.keys[static_cast<i32>(key)];
    }

    bool InputManager::GetKeyUp(Keys key) {
        std::scoped_lock lock(m_InputMutex);
        return !m_InputState.keyboard_current.keys[static_cast<i32>(key)] &&
               m_InputState.keyboard_previous.keys[static_cast<i32>(key)];
    }

    bool InputManager::GetKey(Keys key) {
        std::scoped_lock lock(m_InputMutex);
        return m_InputState.keyboard_current.keys[static_cast<i32>(key)] &&
               m_InputState.keyboard_previous.keys[static_cast<i32>(key)];
    }

    bool InputManager::GetMouseButtonDown(MouseButtons button) {
        std::scoped_lock lock(m_InputMutex);
        return m_InputState.mouse_current.buttons[static_cast<i32>(button)] &&
               !m_InputState.mouse_previous.buttons[static_cast<i32>(button)];
    }

    bool InputManager::GetMouseButtonUp(MouseButtons button) {
        std::scoped_lock lock(m_InputMutex);
        return !m_InputState.mouse_current.buttons[static_cast<i32>(button)] &&
               m_InputState.mouse_previous.buttons[static_cast<i32>(button)];
    }

    bool InputManager::GetMouseButton(MouseButtons button) {
        std::scoped_lock lock(m_InputMutex);
        return m_InputState.mouse_current.buttons[static_cast<i32>(button)] &&
               m_InputState.mouse_previous.buttons[static_cast<i32>(button)];
    }

    glm::vec2 InputManager::GetMousePosition() {
        std::scoped_lock lock(m_InputMutex);
        return m_InputState.mouse_current.position;
    }

#ifdef AXLE_TESTING
    void InputManager::SimulateKeyState(Keys key, bool pressed) {
        SetKey(key, pressed);
    }
    void InputManager::SimulateMouseButtonState(MouseButtons button, bool pressed) {
        SetMouseButton(button, pressed);
    }
    void InputManager::SimulateMousePosition(const glm::vec2& position) {
        SetMousePosition(position);
    }
    void InputManager::SimulateMouseWheel(f32 delta) {
        SetMouseWheel(delta);
    }
    void InputManager::SimulateUpdate() {
        Update();
    }
    void InputManager::SimulateReset() {
        m_InputState = InputState();
    }
#endif // AXLE_TESTING

} // namespace Axle
