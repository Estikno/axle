#include "axpch.hpp"

#include "../Types.hpp"
#include "InputManager.hpp"
#include "../Logger/Log.hpp"
#include "../Events/EventHandler.hpp"
#include "Core/Events/Event.hpp"
#include "Core/Input/InputState.hpp"

#include <glm/vec2.hpp>

namespace Axle {
    std::unique_ptr<InputManager> InputManager::s_Instance;

    void InputManager::Init() {
        if (s_Instance != nullptr) {
            AX_CORE_WARN("Init method of the Input Manager has been called a second time. IGNORING");
            return;
        }

        s_Instance = std::make_unique<InputManager>();

        AX_CORE_INFO("Input Manager initialized...");
    }

    void InputManager::ShutDown() {
        s_Instance.reset();
        AX_CORE_INFO("Input Manager deleted...");
    }

    void InputManager::Update() {
        std::unique_lock lock(m_Mutex);

        // Send events if keys are pressed
        for (u16 i = 0; i < static_cast<u16>(Keys::MaxKeys); ++i) {
            if (GetKeyUnsafe(static_cast<Keys>(i))) {
                KeyIsPressedEvent event(static_cast<Keys>(i));
                AX_SUBMIT_EVENT(std::move(event));
            }
        }

        // Same for mouse buttons
        for (u16 i = 0; i < static_cast<u16>(MouseButtons::MaxButtons); ++i) {
            if (GetMouseButtonUnsafe(static_cast<MouseButtons>(i))) {
                MouseButtonIsPressedEvent event(static_cast<MouseButtons>(i));
                AX_SUBMIT_EVENT(std::move(event));
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
        if (pressed)
            AX_SUBMIT_EVENT(KeyPressedEvent(key));
        else
            AX_SUBMIT_EVENT(KeyReleasedEvent(key));
    }

    void InputManager::SetMouseButton(MouseButtons button, bool pressed) {
        std::unique_lock lock(m_Mutex);

        // Only handles if the state of the key has changed
        if (m_InputState.mouse_current.buttons[static_cast<i32>(button)] == pressed)
            return;
        // Update internal state
        m_InputState.mouse_current.buttons[static_cast<i32>(button)] = pressed;

        // Fire off an event informing of the change in state
        if (pressed)
            AX_SUBMIT_EVENT(MouseButtonPressedEvent(button));
        else
            AX_SUBMIT_EVENT(MouseButtonReleasedEvent(button));
    }

    void InputManager::SetMousePosition(const glm::vec2& position) {
        std::unique_lock lock(m_Mutex);

        // Only handles if the state of the key has changed
        if (m_InputState.mouse_current.position == position)
            return;

        // Update internal state
        m_InputState.mouse_current.position = position;

        // Fire off an event informing of the change in state
        MouseMovedEvent event(position.x, position.y);
        AX_SUBMIT_EVENT(std::move(event));
    }

    void InputManager::SetMouseWheel(f32 deltax, f32 deltay) {
        // Fire off an event informing of the change in state
        MouseScrollEvent event(deltax, deltay);
        AX_SUBMIT_EVENT(std::move(event));
    }

    bool InputManager::GetKeyDownImpl(Keys key) const {
        std::shared_lock lock(m_Mutex);
        return m_InputState.keyboard_current.keys[static_cast<u32>(key)] &&
               !m_InputState.keyboard_previous.keys[static_cast<u32>(key)];
    }

    bool InputManager::GetKeyUpImpl(Keys key) const {
        std::shared_lock lock(m_Mutex);
        return !m_InputState.keyboard_current.keys[static_cast<u32>(key)] &&
               m_InputState.keyboard_previous.keys[static_cast<u32>(key)];
    }

    bool InputManager::GetKeyImpl(Keys key) const {
        std::shared_lock lock(m_Mutex);
        return m_InputState.keyboard_current.keys[static_cast<u32>(key)] &&
               m_InputState.keyboard_previous.keys[static_cast<u32>(key)];
    }

    bool InputManager::GetMouseButtonDownImpl(MouseButtons button) const {
        std::shared_lock lock(m_Mutex);
        return m_InputState.mouse_current.buttons[static_cast<u32>(button)] &&
               !m_InputState.mouse_previous.buttons[static_cast<u32>(button)];
    }

    bool InputManager::GetMouseButtonUpImpl(MouseButtons button) const {
        std::shared_lock lock(m_Mutex);
        return !m_InputState.mouse_current.buttons[static_cast<u32>(button)] &&
               m_InputState.mouse_previous.buttons[static_cast<u32>(button)];
    }

    bool InputManager::GetMouseButtonImpl(MouseButtons button) const {
        std::shared_lock lock(m_Mutex);
        return m_InputState.mouse_current.buttons[static_cast<u32>(button)] &&
               m_InputState.mouse_previous.buttons[static_cast<u32>(button)];
    }

    glm::vec2 InputManager::GetMousePositionImpl() const {
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
    void InputManager::SimulateMouseWheel(f32 deltax, f32 deltay) {
        SetMouseWheel(deltax, deltay);
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
