#pragma once

#include "axpch.hpp"
#include "../Types.hpp"

#include "Core/Core.hpp"
#include "InputState.hpp"

#include <glm/vec2.hpp>

namespace Axle {
    class Input {
    public:
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
        AXLE_API static bool GetKeyDown(Keys key);

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
        AXLE_API static bool GetKeyUp(Keys key);

        /**
		* Returns true if the key is currently being pressed.
        * 
		* Unlike GetKeyDown and GetKeyUp, this will return true as long as the key is pressed down.
        *
		* @param key The key to check.
        * 
		* @returns True if the key is currently being pressed, false otherwise.
        */
        AXLE_API static bool GetKey(Keys key);

        /**
		* Same as GetKeyDown, but for mouse buttons.
        * 
		* @param button The mouse button to check.
        * 
		* @returns True if the mouse button was pressed down this frame, false otherwise.
        */
        AXLE_API static bool GetMouseButtonDown(MouseButtons button);

        /**
		* Same as GetKeyUp, but for mouse buttons.
        *
		* @param button The mouse button to check.
        * 
		* @returns True if the mouse button was released this frame, false otherwise.
        */
        AXLE_API static bool GetMouseButtonUp(MouseButtons button);

        /**
		* Same as GetKey, but for mouse buttons.
        * 
		* @param button The mouse button to check.
        *
		* @returns True if the mouse button is currently being pressed, false otherwise.
        */
        AXLE_API static bool GetMouseButton(MouseButtons button);

        /**
		* Gets the current mouse position in screen coordinates.
        * 
		* @returns The current mouse position in screen coordinates.
        */
		AXLE_API static glm::vec2 GetMousePosition();

		// Modify the keys state. This should only be called by the event system and should not be available to the end user.

        /**
		* Sets the state of a key.
        *
        * Additionaly, the function will also invoke an event to the event system informing of the action.
        * 
		* @param key The key to set the state of.
		* @param pressed True if the key is pressed, false otherwise.
        */
        static void SetKey(Keys key, bool pressed);

        /**
		* Sets the state of a mouse button.
        *
        * Additionaly, the function will also invoke an event to the event system informing of the action.
        * 
		* @param button The mouse button to set the state of.
		* @param pressed True if the mouse button is pressed, false otherwise.
        */
		static void SetMouseButton(MouseButtons button, bool pressed);

        /**
		* Sets the mouse position.
        * 
		* @param position The new mouse position in screen coordinates.
        */
		static void SetMousePosition(const glm::vec2& position);

        /**
		* Sets the mouse wheel delta.
        * 
		* @param delta The amount the mouse wheel has been scrolled.
        */
        static void SetMouseWheel(f32 delta);

        /**
		* Updates the input state.
        */
		static void Update();


        // For testing purposes only
#ifdef AXLE_TESTING
		AXLE_TEST_API static void SimulateKeyState(Keys key, bool pressed);
		AXLE_TEST_API static void SimulateMouseButtonState(MouseButtons button, bool pressed);
		AXLE_TEST_API static void SimulateMousePosition(const glm::vec2& position);
		AXLE_TEST_API static void SimulateMouseWheel(f32 delta);
		AXLE_TEST_API static void SimulateUpdate();
        AXLE_TEST_API static void SimulateReset();
#endif // AXLE_TESTING


	private:
		static InputState s_InputState;
    };
}
