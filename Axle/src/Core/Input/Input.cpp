#include "axpch.hpp"

#include "Input.hpp"
#include "../Logger/Log.hpp"
#include "../Events/EventHandler.hpp"
#include "Math/Math_Types.hpp"

namespace Axle {
	InputState Input::s_InputState;

	void Input::Update() {
		s_InputState.keyboard_previous = s_InputState.keyboard_current;
		s_InputState.mouse_previous = s_InputState.mouse_current;
	}

	void Input::SetKey(Keys key, bool pressed) {
		// Only handles if the state of the key has changed
		if (s_InputState.keyboard_current.keys[(int)key] != pressed) {
			// Update internal state
			s_InputState.keyboard_current.keys[(int)key] = pressed;
		}

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
		if (s_InputState.mouse_current.buttons[(int)button] != pressed) {
			// Update internal state
			s_InputState.mouse_current.buttons[(int)button] = pressed;
		}

		// Fire off an event informing of the change in state
		Event* event;

		if (Input::GetMouseButtonDown(button)) event = new Event(EventType::MouseButtonPressed, EventCategory::Input);
		else if (Input::GetMouseButtonUp(button)) event = new Event(EventType::MouseButtonReleased, EventCategory::Input);
		else event = new Event(EventType::MouseButtonIsPressed, EventCategory::Input);

		event->GetContext().u16[0] = (unsigned short)button;

		AX_ADD_EVENT(event);
	}

	void Input::SetMousePosition(const Vector2 &position) {
		// Only handles if the state of the key has changed
		if (s_InputState.mouse_current.position == position) return;

		// Update internal state
		s_InputState.mouse_current.position = position;

		// Fire off an event informing of the change in state
		Event* event = new Event(EventType::MouseMoved, EventCategory::Input);

		event->GetContext().u16[0] = (unsigned short)position.x;
		event->GetContext().u16[1] = (unsigned short)position.y;

		AX_ADD_EVENT(event);
	}

	void Input::SetMouseWheel(float delta) {
		// Fire off an event informing of the change in state
		Event* event = new Event(EventType::MouseScrolled, EventCategory::Input);

		event->GetContext().f32[0] = delta;

		AX_ADD_EVENT(event);
	}

	bool Input::GetKeyDown(Keys key) {
		return s_InputState.keyboard_current.keys[(int)key] && !s_InputState.keyboard_previous.keys[(int)key];
	}

	bool Input::GetKeyUp(Keys key) {
		return !s_InputState.keyboard_current.keys[(int)key] && s_InputState.keyboard_previous.keys[(int)key];
	}

	bool Input::GetKey(Keys key) {
		return s_InputState.keyboard_current.keys[(int)key] && s_InputState.keyboard_previous.keys[(int)key];
	}

	bool Input::GetMouseButtonDown(MouseButtons button) {
		return s_InputState.mouse_current.buttons[(int)button] && !s_InputState.mouse_previous.buttons[(int)button];
	}

	bool Input::GetMouseButtonUp(MouseButtons button) {
		return !s_InputState.mouse_current.buttons[(int)button] && s_InputState.mouse_previous.buttons[(int)button];
	}

	bool Input::GetMouseButton(MouseButtons button) {
		return s_InputState.mouse_current.buttons[(int)button] && s_InputState.mouse_previous.buttons[(int)button];
	}

	Vector2 Input::GetMousePosition() {
		return s_InputState.mouse_current.position;
	}

#ifdef AXLE_TESTING
	void Input::SimulateKeyState(Keys key, bool pressed) {
		SetKey(key, pressed);
	}
	void Input::SimulateMouseButtonState(MouseButtons button, bool pressed) {
		SetMouseButton(button, pressed);
	}
	void Input::SimulateMousePosition(const Vector2 &position) {
		SetMousePosition(position);
	}
	void Input::SimulateMouseWheel(float delta) {
		SetMouseWheel(delta);
	}
	void Input::SimulateUpdate() {
		Update();
	}
	void Input::SimulateReset() {
		s_InputState = InputState();
	}
#endif // AXLE_TESTING

}
