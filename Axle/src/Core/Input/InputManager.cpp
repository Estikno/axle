#include "axpch.hpp"

#include "../Types.hpp"
#include "InputManager.hpp"
#include "../Logger/Log.hpp"
#include "../Events/EventHandler.hpp"
#include "Core/Events/Event.hpp"
#include "Core/Input/InputState.hpp"

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
        std::unique_lock lock(m_Mutex);

        // Send events if keys are pressed
        for (u16 i = 0; i < static_cast<u16>(Keys::MaxKeys); ++i) {
            if (GetKeyUnsafe(static_cast<Keys>(i))) {
                Event event(EventType::KeyIsPressed, EventCategory::Input);
                event.GetContext().raw_data = std::array<u16, 8>{i};
                AX_DISPATCH_EVENT(std::move(event));
            }
        }

        // Same for mouse buttons
        for (u16 i = 0; i < static_cast<u16>(MouseButtons::MAX_BUTTONS); ++i) {
            if (GetMouseButtonUnsafe(static_cast<MouseButtons>(i))) {
                Event event(EventType::MouseButtonIsPressed, EventCategory::Input);
                event.GetContext().raw_data = std::array<u16, 8>{i};
                AX_DISPATCH_EVENT(std::move(event));
            }
        }

        m_InputState.keyboard_previous = m_InputState.keyboard_current;
        m_InputState.mouse_previous = m_InputState.mouse_current;
    }

    void InputManager::SetKey(Keys key, bool pressed) {
        std::unique_lock lock(m_Mutex);

        // Only handles if the state of the key has changed
        if (m_InputState.keyboard_current.keys[static_cast<i32>(key)] == pressed)
            return;
        // Update internal state
        m_InputState.keyboard_current.keys[static_cast<i32>(key)] = pressed;

        // Fire off an event informing of the change in state
        EventType type = pressed ? EventType::KeyPressed : EventType::KeyReleased;
        Event event(type, EventCategory::Input);

        // event->GetContext().u16_values[0] = static_cast<u16>(key);
        event.GetContext().raw_data = std::array<u16, 8>{static_cast<u16>(key)};

        AX_DISPATCH_EVENT(std::move(event));
    }

    void InputManager::SetMouseButton(MouseButtons button, bool pressed) {
        std::unique_lock lock(m_Mutex);

        // Only handles if the state of the key has changed
        if (m_InputState.mouse_current.buttons[static_cast<i32>(button)] == pressed)
            return;
        // Update internal state
        m_InputState.mouse_current.buttons[static_cast<i32>(button)] = pressed;

        // Fire off an event informing of the change in state
        EventType type = pressed ? EventType::MouseButtonPressed : EventType::MouseButtonReleased;
        Event event(type, EventCategory::Input);

        // event->GetContext().u16_values[0] = static_cast<u16>(button);
        event.GetContext().raw_data = std::array<u16, 8>{static_cast<u16>(button)};

        AX_DISPATCH_EVENT(std::move(event));
    }

    void InputManager::SetMousePosition(const glm::vec2& position) {
        std::unique_lock lock(m_Mutex);

        // Only handles if the state of the key has changed
        if (m_InputState.mouse_current.position == position)
            return;

        // Update internal state
        m_InputState.mouse_current.position = position;

        // Fire off an event informing of the change in state
        Event event(EventType::MouseMoved, EventCategory::Input);

        // event->GetContext().u16_values[0] = static_cast<u16>(position.x);
        // event->GetContext().u16_values[1] = static_cast<u16>(position.y);
        event.GetContext().raw_data = std::array<u16, 8>{static_cast<u16>(position.x), static_cast<u16>(position.y)};

        AX_DISPATCH_EVENT(std::move(event));
    }

    void InputManager::SetMouseWheel(f32 delta) {
        // Fire off an event informing of the change in state
        Event event(EventType::MouseScrolled, EventCategory::Input);
        // event->GetContext().f32_values[0] = delta;
        event.GetContext().raw_data = std::array<f32, 4>{static_cast<f32>(delta)};

        AX_DISPATCH_EVENT(std::move(event));
    }

    bool InputManager::GetKeyDown(Keys key) const {
        std::shared_lock lock(m_Mutex);
        return m_InputState.keyboard_current.keys[static_cast<u32>(key)] &&
               !m_InputState.keyboard_previous.keys[static_cast<u32>(key)];
    }

    bool InputManager::GetKeyUp(Keys key) const {
        std::shared_lock lock(m_Mutex);
        return !m_InputState.keyboard_current.keys[static_cast<u32>(key)] &&
               m_InputState.keyboard_previous.keys[static_cast<u32>(key)];
    }

    bool InputManager::GetKey(Keys key) const {
        std::shared_lock lock(m_Mutex);
        return m_InputState.keyboard_current.keys[static_cast<u32>(key)] &&
               m_InputState.keyboard_previous.keys[static_cast<u32>(key)];
    }

    bool InputManager::GetMouseButtonDown(MouseButtons button) const {
        std::shared_lock lock(m_Mutex);
        return m_InputState.mouse_current.buttons[static_cast<u32>(button)] &&
               !m_InputState.mouse_previous.buttons[static_cast<u32>(button)];
    }

    bool InputManager::GetMouseButtonUp(MouseButtons button) const {
        std::shared_lock lock(m_Mutex);
        return !m_InputState.mouse_current.buttons[static_cast<u32>(button)] &&
               m_InputState.mouse_previous.buttons[static_cast<u32>(button)];
    }

    bool InputManager::GetMouseButton(MouseButtons button) const {
        std::shared_lock lock(m_Mutex);
        return m_InputState.mouse_current.buttons[static_cast<u32>(button)] &&
               m_InputState.mouse_previous.buttons[static_cast<u32>(button)];
    }

    glm::vec2 InputManager::GetMousePosition() const {
        std::shared_lock lock(m_Mutex);
        return m_InputState.mouse_current.position;
    }

    bool InputManager::GetKeyUnsafe(Keys key) const {
        return m_InputState.keyboard_current.keys[static_cast<u32>(key)] &&
               m_InputState.keyboard_previous.keys[static_cast<u32>(key)];
    }

    bool InputManager::GetMouseButtonUnsafe(MouseButtons button) const {
        return m_InputState.mouse_current.buttons[static_cast<u32>(button)] &&
               m_InputState.mouse_previous.buttons[static_cast<u32>(button)];
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
        std::unique_lock lock(m_Mutex);
        m_InputState = InputState();
    }
#endif // AXLE_TESTING

} // namespace Axle
