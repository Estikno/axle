#pragma once

#include "axpch.hpp"

#include "Core/Core.hpp"
#include "Math/Math_Types.hpp"
#include "Input.hpp"

namespace Axle {
	struct KeyBoardState {
		bool keys[256] = { false };
	};

	struct MouseState {
		Vector2 position;
		bool buttons[(int)MouseButtons::BUTTON_MAX_BUTTONS];
	};

	struct InputState {
		KeyBoardState keyboard_current;
		KeyBoardState keyboard_previous;
		MouseState mouse_current;
		MouseState mouse_previous;
	};
}
