#include "axpch.hpp"

#include "../Types.hpp"
#include "InputManager.hpp"
#include "../Logger/Log.hpp"
#include "../Events/EventHandler.hpp"
#include "Core/Application.hpp"
#include "Core/Events/Event.hpp"
#include "Core/Input/InputState.hpp"

#include <GLFW/glfw3.h>
#include <glm/vec2.hpp>

namespace Axle {
    std::unique_ptr<InputManager> InputManager::s_Instance;

    void InputManager::Init() {
        if (s_Instance != nullptr) {
            AX_CORE_WARN(LogChannel::Input, "Init method of the Input Manager has been called a second time. IGNORING");
            return;
        }

        s_Instance = std::make_unique<InputManager>();

        AX_CORE_INFO(LogChannel::Input, "Input Manager initialized...");
    }

    void InputManager::ShutDown() {
        s_Instance.reset();
        AX_CORE_INFO(LogChannel::Input, "Input Manager deleted...");
    }

    void InputManager::Update() {
        std::unique_lock lock(m_Mutex);

        for (u16 i = 0; i < static_cast<u16>(Keys::MaxKeys); ++i) {
            // Send events if keys are pressed
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

        m_InputState.m_KeyboardPrevious = m_InputState.m_KeyboardCurrent;
        m_InputState.m_MousePrevious = m_InputState.m_MouseCurrent;
    }

    void InputManager::SetKey(Keys key, bool pressed) {
        std::unique_lock lock(m_Mutex);

        // Only handles if the state of the key has changed
        if (m_InputState.m_KeyboardCurrent.m_Keys[static_cast<u16>(key)] == pressed)
            return;
        // Update internal state
        m_InputState.m_KeyboardCurrent.m_Keys[static_cast<u16>(key)] = pressed;

        // Fire off an event informing of the change in state
        if (pressed)
            AX_SUBMIT_EVENT(KeyPressedEvent(key));
        else
            AX_SUBMIT_EVENT(KeyReleasedEvent(key));

        // NOTE: glfw needs to be initialized
        // Set multiple taps if it's the case
        if (pressed) {
            TapInfo& ti = m_InputState.m_KeyTaps[static_cast<u16>(key)];

            f32 current = static_cast<f32>(glfwGetTime());
            f32 dt = current - ti.last;

            // The first press does not check anything
            if (ti.count == 0) {
                ti.count = 1;
            } else {
                if (dt < m_InputState.m_DtMax) {
                    ti.count++;
                    AX_SUBMIT_EVENT(KeyTappedEvent(key, ti.count));
                } else
                    ti.count = 1;
            }
            ti.last = current;
        }

        // NOTE: glfw needs to be initialized
        // Update sequences
        if (pressed) {
            for (u32 i = 0; i < m_InputState.m_KeySequences.size(); ++i) {
                Sequence<Keys>& seq = m_InputState.m_KeySequences[i];
                // If any button OTHER than the expected button just went down, invalidate the sequence.
                if (seq.m_Buttons[seq.m_iButton] != key) {
                    seq.m_iButton = 0;
                }
                // Otherwise, if the expected button just went down, check dt and update our state appropriately.
                else if (seq.m_Buttons[seq.m_iButton] == key) {
                    // This is the first button in the sequence
                    if (seq.m_iButton == 0) {
                        seq.m_tStart = static_cast<f32>(glfwGetTime());
                        seq.m_iButton++;
                    } else {
                        f32 dt = static_cast<f32>(glfwGetTime()) - seq.m_tStart;

                        if (dt < seq.m_dtMax) {
                            seq.m_iButton++;

                            // Is the sequence complete?
                            if (seq.m_iButton == seq.m_Buttons.size()) {
                                seq.m_iButton = 0; // reset
                                AX_SUBMIT_EVENT(KeySequenceEvent(i));
                            }
                        }
                        // Not fast enough, reset
                        else
                            seq.m_iButton = 0;
                    }
                }
            }
        }
    }

    void InputManager::SetMouseButton(MouseButtons button, bool pressed) {
        std::unique_lock lock(m_Mutex);

        // Only handles if the state of the key has changed
        if (m_InputState.m_MouseCurrent.m_Buttons[static_cast<u32>(button)] == pressed)
            return;
        // Update internal state
        m_InputState.m_MouseCurrent.m_Buttons[static_cast<u32>(button)] = pressed;

        // Fire off an event informing of the change in state
        if (pressed)
            AX_SUBMIT_EVENT(MouseButtonPressedEvent(button));
        else
            AX_SUBMIT_EVENT(MouseButtonReleasedEvent(button));

        // NOTE: glfw needs to be initialized
        // Set multiple taps if it's the case
        if (pressed) {
            TapInfo& ti = m_InputState.m_MouseTaps[static_cast<u16>(button)];

            f32 current = static_cast<f32>(glfwGetTime());
            f32 dt = current - ti.last;

            // The first press does not check anything
            if (ti.count == 0) {
                ti.count = 1;
            } else {
                if (dt < m_InputState.m_DtMax) {
                    ti.count++;
                    AX_SUBMIT_EVENT(MouseButtonTappedEvent(button, ti.count));
                } else
                    ti.count = 1;
            }

            ti.last = current;
        }

        // NOTE: glfw needs to be initialized
        // Update sequences
        if (pressed) {
            for (u32 i = 0; i < m_InputState.m_MouseSequences.size(); ++i) {
                Sequence<MouseButtons>& seq = m_InputState.m_MouseSequences[i];
                // If any button OTHER than the expected button just went down, invalidate the sequence.
                if (seq.m_Buttons[seq.m_iButton] != button) {
                    seq.m_iButton = 0;
                }
                // Otherwise, if the expected button just went down, check dt and update our state appropriately.
                else if (seq.m_Buttons[seq.m_iButton] == button) {
                    // This is the first button in the sequence
                    if (seq.m_iButton == 0) {
                        seq.m_tStart = static_cast<f32>(glfwGetTime());
                        seq.m_iButton++;
                    } else {
                        f32 dt = static_cast<f32>(glfwGetTime()) - seq.m_tStart;

                        if (dt < seq.m_dtMax) {
                            seq.m_iButton++;

                            // Is the sequence complete?
                            if (seq.m_iButton == seq.m_Buttons.size()) {
                                seq.m_iButton = 0; // reset
                                AX_SUBMIT_EVENT(MouseButtonSequenceEvent(i));
                            }
                        }
                        // Not fast enough, reset
                        else
                            seq.m_iButton = 0;
                    }
                }
            }
        }
    }

    void InputManager::SetMousePosition(const glm::vec2& position) {
        std::unique_lock lock(m_Mutex);

        // Only handles if the state of the key has changed
        if (m_InputState.m_MouseCurrent.position == position)
            return;

        // Update internal state
        m_InputState.m_MouseCurrent.position = position;

        // Fire off an event informing of the change in state
        MouseMovedEvent event(position.x, position.y);
        AX_SUBMIT_EVENT(std::move(event));
    }

    void InputManager::SetMouseWheel(f64 xOffset, f64 yOffset) {
        std::unique_lock lock(m_Mutex);

        // Fire off an event informing of the change in state
        MouseScrollEvent event(xOffset, yOffset);
        AX_SUBMIT_EVENT(std::move(event));
    }

    bool InputManager::GetKeyDownImpl(Keys key) const {
        std::shared_lock lock(m_Mutex);
        return m_InputState.m_KeyboardCurrent.m_Keys[static_cast<u32>(key)] &&
               !m_InputState.m_KeyboardPrevious.m_Keys[static_cast<u32>(key)];
    }

    bool InputManager::GetKeyUpImpl(Keys key) const {
        std::shared_lock lock(m_Mutex);
        return !m_InputState.m_KeyboardCurrent.m_Keys[static_cast<u32>(key)] &&
               m_InputState.m_KeyboardPrevious.m_Keys[static_cast<u32>(key)];
    }

    bool InputManager::GetKeyImpl(Keys key) const {
        std::shared_lock lock(m_Mutex);
        return GetKeyUnsafe(key);
    }

    bool InputManager::GetMouseButtonDownImpl(MouseButtons button) const {
        std::shared_lock lock(m_Mutex);
        return GetMouseButtonDownUnsafe(button);
    }

    bool InputManager::GetMouseButtonUpImpl(MouseButtons button) const {
        std::shared_lock lock(m_Mutex);
        return !m_InputState.m_MouseCurrent.m_Buttons[static_cast<u32>(button)] &&
               m_InputState.m_MousePrevious.m_Buttons[static_cast<u32>(button)];
    }

    bool InputManager::GetMouseButtonImpl(MouseButtons button) const {
        std::shared_lock lock(m_Mutex);
        return GetMouseButtonUnsafe(button);
    }

    glm::vec2 InputManager::GetMousePositionImpl() const {
        std::shared_lock lock(m_Mutex);
        return m_InputState.m_MouseCurrent.position;
    }

    glm::vec2 InputManager::GetMousePositionOffsetImpl() const {
        std::shared_lock lock(m_Mutex);
        return m_InputState.m_MouseCurrent.position - m_InputState.m_MousePrevious.position;
    }

    bool InputManager::GetKeyUnsafe(Keys key) const {
        return m_InputState.m_KeyboardCurrent.m_Keys[static_cast<u32>(key)] &&
               m_InputState.m_KeyboardPrevious.m_Keys[static_cast<u32>(key)];
    }

    bool InputManager::GetMouseButtonUnsafe(MouseButtons button) const {
        return m_InputState.m_MouseCurrent.m_Buttons[static_cast<u32>(button)] &&
               m_InputState.m_MousePrevious.m_Buttons[static_cast<u32>(button)];
    }

    bool InputManager::GetKeyDownUnsafe(Keys key) const {
        return m_InputState.m_KeyboardCurrent.m_Keys[static_cast<u32>(key)] &&
               !m_InputState.m_KeyboardPrevious.m_Keys[static_cast<u32>(key)];
    }

    bool InputManager::GetMouseButtonDownUnsafe(MouseButtons button) const {
        return m_InputState.m_MouseCurrent.m_Buttons[static_cast<u32>(button)] &&
               !m_InputState.m_MousePrevious.m_Buttons[static_cast<u32>(button)];
    }

    bool InputManager::IsKeyTappedNTimesImpl(Keys key, u8 times) const {
        std::shared_lock lock(m_Mutex);
        return m_InputState.m_KeyTaps[static_cast<u16>(key)].count == times;
    }

    bool InputManager::IsKeyDoubleClickedImpl(Keys key) const {
        return IsKeyTappedNTimesImpl(key, 2);
    }

    bool InputManager::IsMouseButtonTappedNTimesImpl(MouseButtons button, u8 times) const {
        std::shared_lock lock(m_Mutex);
        return m_InputState.m_MouseTaps[static_cast<u16>(button)].count == times;
    }

    bool InputManager::IsMouseButtonDoubleClickedImpl(MouseButtons button) const {
        return IsMouseButtonTappedNTimesImpl(button, 2);
    }

    u32 InputManager::DefineKeySequenceImpl(const std::vector<Keys>& sec, f32 dtMax) {
        std::unique_lock lock(m_Mutex);
        m_InputState.m_KeySequences.push_back({.m_Buttons = sec, .m_dtMax = dtMax, .m_iButton = 0, .m_tStart = 0});
        return m_InputState.m_KeySequences.size() - 1;
    }

    u32 InputManager::DefineMouseButtonSequenceImpl(const std::vector<MouseButtons>& sec, f32 dtMax) {
        std::unique_lock lock(m_Mutex);
        m_InputState.m_MouseSequences.push_back({.m_Buttons = sec, .m_dtMax = dtMax, .m_iButton = 0, .m_tStart = 0});
        return m_InputState.m_MouseSequences.size() - 1;
    }

    void InputManager::SetCursorModeImpl(CursorMode mode) const {
        const i32 glfwEquivalent = static_cast<i32>(mode) + CursorModeOffset;
        glfwSetInputMode(Application::GetInstance().GetWindow().GetNativeWindow(), GLFW_CURSOR, glfwEquivalent);
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
