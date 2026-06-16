#pragma once

#include "axpch.hpp"

#include "../Types.hpp"
#include "Core/Core.hpp"

#include <glm/vec2.hpp>

// Key codes taken from GLFW
namespace Axle {
    enum class MouseButtons {
        Left = 0,
        Right = 1,
        Middle = 2,

        Button4 = 3,
        Button5 = 4,
        Button6 = 5,
        Button7 = 6,
        Button8 = 7,

        MaxButtons
    };

    enum class Keys {
        Unknown = -1,

        // Printable keys
        Space = 32,
        Apostrophe = 39, /* ' */
        Comma = 44,      /* , */
        Minus = 45,      /* - */
        Period = 46,     /* . */
        Slash = 47,      /* / */
        Num0 = 48,
        Num1 = 49,
        Num2 = 50,
        Num3 = 51,
        Num4 = 52,
        Num5 = 53,
        Num6 = 54,
        Num7 = 55,
        Num8 = 56,
        Num9 = 57,
        Semicolon = 59, /* ; */
        Equal = 61,     /* = */
        A = 65,
        B = 66,
        C = 67,
        D = 68,
        E = 69,
        F = 70,
        G = 71,
        H = 72,
        I = 73,
        J = 74,
        K = 75,
        L = 76,
        M = 77,
        N = 78,
        O = 79,
        P = 80,
        Q = 81,
        R = 82,
        S = 83,
        T = 84,
        U = 85,
        V = 86,
        W = 87,
        X = 88,
        Y = 89,
        Z = 90,
        LeftBracket = 91,  /* [ */
        Backslash = 92,    /* \ */
        RightBracket = 93, /* ] */
        GraveAccent = 96,  /* ` */
        World1 = 161,      /* non-US #1 */
        World2 = 162,      /* non-US #2 */

        // Function keys
        Escape = 256,
        Enter = 257,
        Tab = 258,
        Backspace = 259,
        Insert = 260,
        Delete = 261,
        Right = 262,
        Left = 263,
        Down = 264,
        Up = 265,
        PageUp = 266,
        PageDown = 267,
        Home = 268,
        End = 269,
        CapsLock = 280,
        ScrollLock = 281,
        NumLock = 282,
        PrintScreen = 283,
        Pause = 284,
        F1 = 290,
        F2 = 291,
        F3 = 292,
        F4 = 293,
        F5 = 294,
        F6 = 295,
        F7 = 296,
        F8 = 297,
        F9 = 298,
        F10 = 299,
        F11 = 300,
        F12 = 301,
        F13 = 302,
        F14 = 303,
        F15 = 304,
        F16 = 305,
        F17 = 306,
        F18 = 307,
        F19 = 308,
        F20 = 309,
        F21 = 310,
        F22 = 311,
        F23 = 312,
        F24 = 313,
        F25 = 314,

        // Keypad
        Numpad0 = 320,
        Numpad1 = 321,
        Numpad2 = 322,
        Numpad3 = 323,
        Numpad4 = 324,
        Numpad5 = 325,
        Numpad6 = 326,
        Numpad7 = 327,
        Numpad8 = 328,
        Numpad9 = 329,
        NumpadDecimal = 330,
        NumpadDivide = 331,
        NumpadMultiply = 332,
        NumpadSubtract = 333,
        NumpadAdd = 334,
        NumpadEnter = 335,
        NumpadEqual = 336,

        // Modifier keys
        LShift = 340,
        LControl = 341,
        LAlt = 342,
        LSuper = 343,
        RShift = 344,
        RControl = 345,
        RAlt = 346,
        RSuper = 347,
        Menu = 348,

        MaxKeys
    };

    // Basic info for detecting multiple taps for a specific key/button
    struct TapInfo {
        // NOTE: Consider changing this to a double to mitigate precision issues over a long period
        f32 last = 0.0f; // Last press down event, in seconds
        u8 count = 0;    // Stores the ammount of taps currently recorded
    };

    template <typename T>
    struct Sequence {
        std::vector<T> m_Buttons; // sequence of buttons to watch for
        f32 m_dtMax;              // max time for entire sequence
        u32 m_iButton;            // next button to watch for in seq.
        f32 m_tStart;             // start time of sequence, in seconds
    };

    struct KeyBoardState {
        // 0 == not pressed, 1 == pressed
        std::bitset<static_cast<u32>(Keys::MaxKeys)> m_Keys;
    };

    struct MouseState {
        glm::vec2 position;
        // 0 == not pressed, 1 == pressed
        std::bitset<static_cast<u32>(MouseButtons::MaxButtons)> m_Buttons;
    };

    struct InputState {
        KeyBoardState m_KeyboardCurrent;
        KeyBoardState m_KeyboardPrevious;
        MouseState m_MouseCurrent;
        MouseState m_MousePrevious;

        // Max allowed time between multiple tap presses
        f32 m_DtMax = 0.5f;
        std::array<TapInfo, static_cast<u32>(Keys::MaxKeys)> m_KeyTaps{};
        std::array<TapInfo, static_cast<u32>(MouseButtons::MaxButtons)> m_MouseTaps{};

        std::vector<Sequence<Keys>> m_KeySequences;
        std::vector<Sequence<MouseButtons>> m_MouseSequences;
    };
} // namespace Axle
