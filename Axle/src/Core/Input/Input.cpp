#include "axpch.hpp"

#include "Input.hpp"
#include "../Logger/Log.hpp"
#include "../Events/EventHandler.hpp"

namespace Axle {
	void Input::Update() {
		s_inputState.keyboard_previous = s_inputState.keyboard_current;
	}

	void Input::SetKey(Keys key, bool pressed) {
		if (s_inputState.keyboard_current.keys[(int)key] != pressed) {
			s_inputState.keyboard_current.keys[(int)key] = pressed;

			// TODO: Add an event that notifies the other parts of the program about the key press
		}
	}
}
