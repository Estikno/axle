#include "axpch.hpp"

#include "Input.hpp"
#include "../Logger/Log.hpp"
#include "../Events/EventHandler.hpp"

namespace Axle {
	InputState Input::s_inputState;

	void Input::Update() {
		s_inputState.keyboard_previous = s_inputState.keyboard_current;
		s_inputState.mouse_previous = s_inputState.mouse_current;
	}

	void Input::SetKey(Keys key, bool pressed) {
		// Only handles if the state of the key has changed
		if (s_inputState.keyboard_current.keys[(int)key] == pressed) return;

		// Update internal state
		s_inputState.keyboard_current.keys[(int)key] = pressed;

		if (!pressed) return;

		// Fire off an event informing of the change in state
		Event* event;

		if (Input::GetKeyDown(key)) event = new Event(EventType::KeyPressed, EventCategory::Input);
		else if (Input::GetKeyUp(key)) event = new Event(EventType::KeyReleased, EventCategory::Input);
		else event = new Event(EventType::KeyIsPressed, EventCategory::Input);

		event->GetContext().u16[0] = (unsigned short)key;

		AX_ADD_EVENT(event);
	}

	void Input::SetMouseButton(MouseButtons button, bool pressed) {
		// Only handles if the state of the key has changed
		if (s_inputState.mouse_current.buttons[(int)button] == pressed) return;

		// Update internal state
		s_inputState.mouse_current.buttons[(int)button] = pressed;

		if (!pressed) return;

		// Fire off an event informing of the change in state
		Event* event;

		if (Input::GetMouseButtonDown(button)) event = new Event(EventType::MouseButtonPressed, EventCategory::Input);
		else if (Input::GetMouseButtonUp(button)) event = new Event(EventType::MouseButtonReleased, EventCategory::Input);
		else event = new Event(EventType::MouseButtonIsPressed, EventCategory::Input);

		event->GetContext().u16[0] = (unsigned short)button;

		AX_ADD_EVENT(event);
	}

	void Input::SetMousePosition(Vector2 position) {
		// Only handles if the state of the key has changed
		if (s_inputState.mouse_current.position == position) return;

		// Update internal state
		s_inputState.mouse_current.position = position;

		// Fire off an event informing of the change in state
		Event* event = new Event(EventType::MouseMoved, EventCategory::Input);

		event->GetContext().u16[0] = (unsigned short)position.x;
		event->GetContext().u16[1] = (unsigned short)position.y;

		AX_ADD_EVENT(event);
	}

	void Input::SetMouseWheel(float delta) {
		// Only handles if the state of the key has changed
		if (Mathf::Approximately(s_inputState.mouse_current.wheel_delta, delta)) return;

		s_inputState.mouse_current.wheel_delta = delta;

		// Fire off an event informing of the change in state
		Event* event = new Event(EventType::MouseScrolled, EventCategory::Input);

		event->GetContext().u16[0] = (unsigned short)delta;

		AX_ADD_EVENT(event);
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
