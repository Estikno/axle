#include "axpch.hpp"

#include "Input.hpp"
#include "../Logger/Log.hpp"
#include "../Events/EventHandler.hpp"

namespace Axle {
	InputState Input::s_inputState;

	void Input::Update() {
		s_inputState.keyboard_previous = s_inputState.keyboard_current;
	}

	void Input::SetKey(Keys key, bool pressed) {
		if (s_inputState.keyboard_current.keys[(int)key] != pressed) {
			s_inputState.keyboard_current.keys[(int)key] = pressed;

			// TODO: Add an event that notifies the other parts of the program about the key press
			Event* event;

			if (Input::GetKeyDown(key)) event = new Event(EventType::KeyPressed, EventCategory::Input);
			else if (Input::GetKeyUp(key)) event = new Event(EventType::KeyReleased, EventCategory::Input);
			else event = new Event(EventType::KeyIsPressed, EventCategory::Input);

			AX_ADD_EVENT(event);
		}
	}

	void Input::SetMouseButton(MouseButtons button, bool pressed) {
		if (s_inputState.mouse_current.buttons[(int)button] != pressed) {
			s_inputState.mouse_current.buttons[(int)button] = pressed;

			// TODO: Add an event that notifies the other parts of the program about the key press
		}
	}

	void Input::SetMousePosition(Vector2 position) {
		if (s_inputState.mouse_current.position != position) {
			s_inputState.mouse_previous.position = s_inputState.mouse_current.position;

			// TODO: Add an event that notifies the other parts of the program about the key press
		}
	}

	void Input::SetMouseWheel(float delta) {
		if (!Mathf::Approximately(s_inputState.mouse_current.wheel_delta, delta)) {
			s_inputState.mouse_current.wheel_delta = delta;

			// TODO: Add an event that notifies the other parts of the program about the key press
		}
	}

	bool Input::GetKeyDown(Keys key) {
		return s_inputState.keyboard_current.keys[(int)key] && !s_inputState.keyboard_previous.keys[(int)key];
	}

	bool Input::GetKeyUp(Keys key) {
		return !s_inputState.keyboard_current.keys[(int)key] && s_inputState.keyboard_previous.keys[(int)key];
	}

	bool Input::GetKey(Keys key) {
		return s_inputState.keyboard_current.keys[(int)key];
	}

	bool Input::GetMouseButtonDown(MouseButtons button) {
		return s_inputState.mouse_current.buttons[(int)button] && !s_inputState.mouse_previous.buttons[(int)button];
	}

	bool Input::GetMouseButtonUp(MouseButtons button) {
		return !s_inputState.mouse_current.buttons[(int)button] && s_inputState.mouse_previous.buttons[(int)button];
	}

	bool Input::GetMouseButton(MouseButtons button) {
		return s_inputState.mouse_current.buttons[(int)button];
	}

	Vector2 Input::GetMousePosition() {
		return s_inputState.mouse_current.position;
	}
}
