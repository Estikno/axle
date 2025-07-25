#pragma once

#include "axpch.hpp"

#include "Core/Core.hpp"
#include "Math/Math_Types.hpp"

#include <GLFW/glfw3.h>

namespace Axle {
	enum class MouseButtons {
		Unknown = -1,

		BUTTON_LEFT,
		BUTTON_RIGHT,
		BUTTON_MIDDLE,

		BUTTON_MAX_BUTTONS
	};

	enum class Keys {
		Unknown = -1,

		Backspace = 0x08,
		Enter = 0x0D,
		Tab = 0x09,
		Shift = 0x10,
		Control = 0x11,

		Pause = 0x13,
		Capital = 0x14,

		Escape = 0x1B,

		Convert = 0x1C,
		NonConvert = 0x1D,
		Accept = 0x1E,
		ModeChange = 0x1F,

		Space = 0x20,
		Prior = 0x21,
		Next = 0x22,
		End = 0x23,
		Home = 0x24,
		Left = 0x25,
		Up = 0x26,
		Right = 0x27,
		Down = 0x28,
		Select = 0x29,
		Print = 0x2A,
		Execute = 0x2B,
		Snapshot = 0x2C,
		Insert = 0x2D,
		Delete = 0x2E,
		Help = 0x2F,

		A = 0x41,
		B = 0x42,
		C = 0x43,
		D = 0x44,
		E = 0x45,
		F = 0x46,
		G = 0x47,
		H = 0x48,
		I = 0x49,
		J = 0x4A,
		K = 0x4B,
		L = 0x4C,
		M = 0x4D,
		N = 0x4E,
		O = 0x4F,
		P = 0x50,
		Q = 0x51,
		R = 0x52,
		S = 0x53,
		T = 0x54,
		U = 0x55,
		V = 0x56,
		W = 0x57,
		X = 0x58,
		Y = 0x59,
		Z = 0x5A,

		LWin = 0x5B,
		RWin = 0x5C,
		Apps = 0x5D,

		Sleep = 0x5F,

		Numpad0 = 0x60,
		Numpad1 = 0x61,
		Numpad2 = 0x62,
		Numpad3 = 0x63,
		Numpad4 = 0x64,
		Numpad5 = 0x65,
		Numpad6 = 0x66,
		Numpad7 = 0x67,
		Numpad8 = 0x68,
		Numpad9 = 0x69,
		Multiply = 0x6A,
		Add = 0x6B,
		Separator = 0x6C,
		Subtract = 0x6D,
		Decimal = 0x6E,
		Divide = 0x6F,

		F1 = 0x70,
		F2 = 0x71,
		F3 = 0x72,
		F4 = 0x73,
		F5 = 0x74,
		F6 = 0x75,
		F7 = 0x76,
		F8 = 0x77,
		F9 = 0x78,
		F10 = 0x79,
		F11 = 0x7A,
		F12 = 0x7B,
		F13 = 0x7C,
		F14 = 0x7D,
		F15 = 0x7E,
		F16 = 0x7F,
		F17 = 0x80,
		F18 = 0x81,
		F19 = 0x82,
		F20 = 0x83,
		F21 = 0x84,
		F22 = 0x85,
		F23 = 0x86,
		F24 = 0x87,

		NumLock = 0x90,
		Scroll = 0x91,

		NumpadEqual = 0x92,

		LShift = 0xA0,
		RShift = 0xA1,
		LControl = 0xA2,
		RControl = 0xA3,
		LMenu = 0xA4,
		RMenu = 0xA5,

		Semicolon = 0xBA,
		Plus = 0xBB,
		Comma = 0xBC,
		Minus = 0xBD,
		Period = 0xBE,
		Slash = 0xBF,
		Grave = 0xC0,

		MaxKeys
	};

	struct KeyBoardState {
		bool keys[256] = { false };
	};

	struct MouseState {
		Vector2 position;
		bool buttons[(int)MouseButtons::BUTTON_MAX_BUTTONS];
		float wheel_delta = 0.0f;
	};

	struct InputState {
		KeyBoardState keyboard_current;
		KeyBoardState keyboard_previous;
		MouseState mouse_current;
		MouseState mouse_previous;
	};

	static Keys ConvertGLFWKeys(int glfw_key) {
		switch (glfw_key) {
		case GLFW_KEY_BACKSPACE: return Keys::Backspace;
		case GLFW_KEY_ENTER: return Keys::Enter;
		case GLFW_KEY_TAB: return Keys::Tab;
		case GLFW_KEY_LEFT_SHIFT: return Keys::LShift;
		case GLFW_KEY_RIGHT_SHIFT: return Keys::RShift;
		case GLFW_KEY_LEFT_CONTROL: return Keys::LControl;
		case GLFW_KEY_RIGHT_CONTROL: return Keys::RControl;
		case GLFW_KEY_LEFT_ALT: return Keys::LMenu;
		case GLFW_KEY_RIGHT_ALT: return Keys::RMenu;
		case GLFW_KEY_LEFT_SUPER: return Keys::LWin;
		case GLFW_KEY_RIGHT_SUPER: return Keys::RWin;
		case GLFW_KEY_ESCAPE: return Keys::Escape;
		case GLFW_KEY_SPACE: return Keys::Space;
		case GLFW_KEY_PAGE_UP: return Keys::Prior;
		case GLFW_KEY_PAGE_DOWN: return Keys::Next;
		case GLFW_KEY_END: return Keys::End;
		case GLFW_KEY_HOME: return Keys::Home;
		case GLFW_KEY_LEFT: return Keys::Left;
		case GLFW_KEY_UP: return Keys::Up;
		case GLFW_KEY_RIGHT: return Keys::Right;
		case GLFW_KEY_DOWN: return Keys::Down;
		case GLFW_KEY_INSERT: return Keys::Insert;
		case GLFW_KEY_DELETE: return Keys::Delete;
		case GLFW_KEY_CAPS_LOCK: return Keys::Capital;
		case GLFW_KEY_PRINT_SCREEN: return Keys::Snapshot;
		case GLFW_KEY_SCROLL_LOCK: return Keys::Scroll;
		case GLFW_KEY_PAUSE: return Keys::Pause;
		case GLFW_KEY_MENU: return Keys::Apps;
		case GLFW_KEY_KP_0: return Keys::Numpad0;
		case GLFW_KEY_KP_1: return Keys::Numpad1;
		case GLFW_KEY_KP_2: return Keys::Numpad2;
		case GLFW_KEY_KP_3: return Keys::Numpad3;
		case GLFW_KEY_KP_4: return Keys::Numpad4;
		case GLFW_KEY_KP_5: return Keys::Numpad5;
		case GLFW_KEY_KP_6: return Keys::Numpad6;
		case GLFW_KEY_KP_7: return Keys::Numpad7;
		case GLFW_KEY_KP_8: return Keys::Numpad8;
		case GLFW_KEY_KP_9: return Keys::Numpad9;
		case GLFW_KEY_KP_ADD: return Keys::Add;
		case GLFW_KEY_KP_SUBTRACT: return Keys::Subtract;
		case GLFW_KEY_KP_MULTIPLY: return Keys::Multiply;
		case GLFW_KEY_KP_DIVIDE: return Keys::Divide;
		case GLFW_KEY_KP_DECIMAL: return Keys::Decimal;
		case GLFW_KEY_KP_EQUAL: return Keys::NumpadEqual;

		case GLFW_KEY_F1: return Keys::F1;
		case GLFW_KEY_F2: return Keys::F2;
		case GLFW_KEY_F3: return Keys::F3;
		case GLFW_KEY_F4: return Keys::F4;
		case GLFW_KEY_F5: return Keys::F5;
		case GLFW_KEY_F6: return Keys::F6;
		case GLFW_KEY_F7: return Keys::F7;
		case GLFW_KEY_F8: return Keys::F8;
		case GLFW_KEY_F9: return Keys::F9;
		case GLFW_KEY_F10: return Keys::F10;
		case GLFW_KEY_F11: return Keys::F11;
		case GLFW_KEY_F12: return Keys::F12;

		case GLFW_KEY_A: return Keys::A;
		case GLFW_KEY_B: return Keys::B;
		case GLFW_KEY_C: return Keys::C;
		case GLFW_KEY_D: return Keys::D;
		case GLFW_KEY_E: return Keys::E;
		case GLFW_KEY_F: return Keys::F;
		case GLFW_KEY_G: return Keys::G;
		case GLFW_KEY_H: return Keys::H;
		case GLFW_KEY_I: return Keys::I;
		case GLFW_KEY_J: return Keys::J;
		case GLFW_KEY_K: return Keys::K;
		case GLFW_KEY_L: return Keys::L;
		case GLFW_KEY_M: return Keys::M;
		case GLFW_KEY_N: return Keys::N;
		case GLFW_KEY_O: return Keys::O;
		case GLFW_KEY_P: return Keys::P;
		case GLFW_KEY_Q: return Keys::Q;
		case GLFW_KEY_R: return Keys::R;
		case GLFW_KEY_S: return Keys::S;
		case GLFW_KEY_T: return Keys::T;
		case GLFW_KEY_U: return Keys::U;
		case GLFW_KEY_V: return Keys::V;
		case GLFW_KEY_W: return Keys::W;
		case GLFW_KEY_X: return Keys::X;
		case GLFW_KEY_Y: return Keys::Y;
		case GLFW_KEY_Z: return Keys::Z;

		case GLFW_KEY_SEMICOLON: return Keys::Semicolon;
		case GLFW_KEY_EQUAL: return Keys::Plus;
		case GLFW_KEY_COMMA: return Keys::Comma;
		case GLFW_KEY_MINUS: return Keys::Minus;
		case GLFW_KEY_PERIOD: return Keys::Period;
		case GLFW_KEY_SLASH: return Keys::Slash;
		case GLFW_KEY_GRAVE_ACCENT: return Keys::Grave;

		default: return Keys::Unknown;
		}
	}

	static MouseButtons ConvertGLFWMouseButtons(int glfw_button) {
		switch (glfw_button) {
		case GLFW_MOUSE_BUTTON_LEFT: return MouseButtons::BUTTON_LEFT;
		case GLFW_MOUSE_BUTTON_RIGHT: return MouseButtons::BUTTON_RIGHT;
		case GLFW_MOUSE_BUTTON_MIDDLE: return MouseButtons::BUTTON_MIDDLE;
		default: return MouseButtons::Unknown;
		}
	}
}
