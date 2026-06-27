#pragma once

#include "axpch.hpp"

#include "../Types.hpp"
#include "Core/Core.hpp"
#include "InputState.hpp"

#include <glm/vec2.hpp>

namespace Axle {
    class AXLE_API InputManager {
    public:
        InputManager(const InputManager&) = delete;
        InputManager& operator=(const InputManager&) = delete;

        // Constructors do nothing because the initialization/destruction is manual with Init/ShutDown
        InputManager() {}
        ~InputManager() {}

        /**
         * Initializes the Input manager and its singleton
         *
         * Important: This has to be called before using the macros and any other functionality
         * This function is NOT thread safe so it must be called from only one thread and only once to be safe.
         */
        static void Init();

        /**
         * Shutdowns the manager, important to call when no other component depends on it anymore
         * This function is NOT thread safe so it must be called from only one thread and only once to be safe.
         */
        static void ShutDown();

        /**
         * Gets the Input Manager singleton
         *
         * The manager has to have already been initilized before getting the instance.
         *
         * @returns Returns a reference to the Event Handler
         */
        inline static InputManager& GetInstance() noexcept {
            return *s_Instance;
        }

        /**
         * Returns true if the key was pressed down this frame.
         *
         * This will only return true once per key press.
         * If you want to know if a key is being pressed use GetKey instead.
         *
         * @param key The key to check.
         *
         * @returns True if the key was pressed down this frame, false otherwise.
         */
        inline static bool GetKeyDown(Keys key) {
            return s_Instance->GetKeyDownImpl(key);
        }

        /**
         * Returns true if the key was released this frame.
         *
         * Just as GetKeyDown, this will only return true once per key release.
         * If you want to know if a key is not being pressed use GetKey instead.
         *
         * @param key The key to check.
         *
         * @returns True if the key was released this frame, false otherwise.
         */
        inline static bool GetKeyUp(Keys key) {
            return s_Instance->GetKeyUpImpl(key);
        }

        /**
         * Returns true if the key is currently being pressed.
         *
         * Unlike GetKeyDown and GetKeyUp, this will return true as long as the key is pressed down.
         *
         * @param key The key to check.
         *
         * @returns True if the key is currently being pressed, false otherwise.
         */
        inline static bool GetKey(Keys key) {
            return s_Instance->GetKeyImpl(key);
        }

        /**
         * Same as GetKeyDown, but for mouse buttons.
         *
         * @param button The mouse button to check.
         *
         * @returns True if the mouse button was pressed down this frame, false otherwise.
         */
        inline static bool GetMouseButtonDown(MouseButtons button) {
            return s_Instance->GetMouseButtonDownImpl(button);
        }

        /**
         * Same as GetKeyUp, but for mouse buttons.
         *
         * @param button The mouse button to check.
         *
         * @returns True if the mouse button was released this frame, false otherwise.
         */
        inline static bool GetMouseButtonUp(MouseButtons button) {
            return s_Instance->GetMouseButtonUpImpl(button);
        }

        /**
         * Same as GetKey, but for mouse buttons.
         *
         * @param button The mouse button to check.
         *
         * @returns True if the mouse button is currently being pressed, false otherwise.
         */
        inline static bool GetMouseButton(MouseButtons button) {
            return s_Instance->GetMouseButtonImpl(button);
        }

        /**
         * Gets the current mouse position in screen coordinates.
         *
         * @returns The current mouse position in screen coordinates.
         */
        inline static glm::vec2 GetMousePosition() {
            return s_Instance->GetMousePositionImpl();
        }

        /**
         * Gets the mouse wheel offset
         *
         * @returns A pair which contains the offset in the x/horizontal axis first and the vertical/y offset second
         */
        inline static std::pair<f64, f64> GetMouseWheelOffset() {
            return s_Instance->GetMouseWheelOffsetImpl();
        }

        /**
         * Gets the mouse position offset
         *
         * @returns A vec2 which contains the two (x and y) position offsets
         */
        inline static glm::vec2 GetMousePositionOffset() {
            return s_Instance->GetMousePositionOffsetImpl();
        }

        /**
         * Checks if the given key has been tappend n times repeatedly
         *
         * @param key The key to check
         * @param times How many times has to have been pressed
         *
         * @returns true if the key has been pressed the amout of times or more
         * */
        inline static bool IsKeyTappedNTimes(Keys key, u8 times) {
            return s_Instance->IsKeyTappedNTimesImpl(key, times);
        }

        /**
         * Checks if the given key has been double clicked
         * This method is a specialization of the general IsKeyTappedNTimes function.
         *
         * @param key The key to check
         *
         * @returns true if the key has been double clicked
         * */
        inline static bool IsKeyDoubleClicked(Keys key) {
            return s_Instance->IsKeyDoubleClickedImpl(key);
        }

        /**
         * Checks if the given mouse button has been tappend n times repeatedly
         *
         * @param button The mouse button to check
         * @param times How many times has to have been pressed
         *
         * @returns true if the mouse button has been pressed the amout of times or more
         * */
        inline static bool IsMouseButtonTappedNTimes(MouseButtons button, u8 times) {
            return s_Instance->IsMouseButtonTappedNTimesImpl(button, times);
        }

        /**
         * Checks if the given mouse button has been double clicked
         * This method is a specialization of the general IsMouseButtonTappedNTimes function.
         *
         * @param button The mouse button to check
         *
         * @returns true if the mouse button has been double clicked
         * */
        inline static bool IsMouseButtonDoubleClicked(MouseButtons button) {
            return s_Instance->IsMouseButtonDoubleClickedImpl(button);
        }

        /**
         * Defines a sequence of keys and returns an id to the newly created sequence
         * Sequences can only be checked via events.
         *
         * @param sec The sequence of keys you want to keep track of
         * @param dtMax The maximum time for the entire sequence
         *
         * @returns An id that identifies the newly created key sequence
         * */
        inline static u32 DefineKeySequence(const std::vector<Keys>& sec, f32 dtMax) {
            return s_Instance->DefineKeySequenceImpl(sec, dtMax);
        }

        /**
         * Defines a sequence of mouse buttons and returns an id to the newly created sequence
         * Sequences can only be checked via events.
         *
         * @param sec The sequence of mouse button presses you want to keep track of
         * @param dtMax The maximum time for the entire sequence
         *
         * @returns An id that identifies the newly created mouse button sequence
         * */
        inline static u32 DefineMouseButtonSequence(const std::vector<MouseButtons>& sec, f32 dtMax) {
            return s_Instance->DefineMouseButtonSequenceImpl(sec, dtMax);
        }


        // Modify the keys state. This should only be called by the event system and should not be available to the end
        // user.

        /**
         * Sets the state of a key.
         *
         * Additionaly, the function will also invoke an event to the event system informing of the action.
         *
         * @param key The key to set the state of.
         * @param pressed True if the key is pressed, false otherwise.
         */
        void SetKey(Keys key, bool pressed);

        /**
         * Sets the state of a mouse button.
         *
         * Additionaly, the function will also invoke an event to the event system informing of the action.
         *
         * @param button The mouse button to set the state of.
         * @param pressed True if the mouse button is pressed, false otherwise.
         */
        void SetMouseButton(MouseButtons button, bool pressed);

        /**
         * Sets the mouse position.
         *
         * @param position The new mouse position in screen coordinates.
         */
        void SetMousePosition(const glm::vec2& position);

        /**
         * Sets the mouse wheel delta.
         *
         * @param delta The amount the mouse wheel has been scrolled.
         */
        void SetMouseWheel(f64 xOffset, f64 yOffset);

        /**
         * Updates the input state.
         */
        void Update();


        // For testing purposes only
#ifdef AXLE_TESTING
        void SimulateKeyState(Keys key, bool pressed);
        void SimulateMouseButtonState(MouseButtons button, bool pressed);
        void SimulateMousePosition(const glm::vec2& position);
        void SimulateMouseWheel(f32 deltax, f32 deltay);
        void SimulateUpdate();
        void SimulateReset();
#endif // AXLE_TESTING

    private:
        // Implementations of the static methods
        bool GetKeyDownImpl(Keys key) const;
        bool GetKeyUpImpl(Keys key) const;
        bool GetKeyImpl(Keys key) const;
        bool GetMouseButtonDownImpl(MouseButtons button) const;
        bool GetMouseButtonUpImpl(MouseButtons button) const;
        bool GetMouseButtonImpl(MouseButtons button) const;
        glm::vec2 GetMousePositionImpl() const;
        bool IsKeyTappedNTimesImpl(Keys key, u8 times) const;
        bool IsKeyDoubleClickedImpl(Keys key) const;
        bool IsMouseButtonTappedNTimesImpl(MouseButtons button, u8 times) const;
        bool IsMouseButtonDoubleClickedImpl(MouseButtons button) const;
        u32 DefineKeySequenceImpl(const std::vector<Keys>& sec, f32 dtMax);
        u32 DefineMouseButtonSequenceImpl(const std::vector<MouseButtons>& sec, f32 dtMax);
        std::pair<f64, f64> GetMouseWheelOffsetImpl() const;
        glm::vec2 GetMousePositionOffsetImpl() const;

        // Unsafe Implementations
        bool GetKeyUnsafe(Keys key) const;
        bool GetMouseButtonUnsafe(MouseButtons button) const;
        bool GetKeyDownUnsafe(Keys key) const;
        bool GetMouseButtonDownUnsafe(MouseButtons button) const;

        static std::unique_ptr<InputManager> s_Instance;

        InputState m_InputState{};
        mutable std::shared_mutex m_Mutex;
    };
} // namespace Axle
