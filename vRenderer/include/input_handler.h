#pragma once

#include <GLFW/glfw3.h>
#include <stdint.h>
#include <vector>
#include <functional>
#include "Event.h"

namespace inp
{
    using InputState = uint64_t;

    enum InputKeys : InputState {
        NONE = 0,
        // Mouse buttons
        MOUSE_LEFT = 1ull << 0,
        MOUSE_RIGHT = 1ull << 1,
        MOUSE_MIDDLE = 1ull << 2,
        // Common keyboard controls
        KEY_W = 1ull << 3,
        KEY_A = 1ull << 4,
        KEY_S = 1ull << 5,
        KEY_D = 1ull << 6,
        KEY_Q = 1ull << 7,
        KEY_E = 1ull << 8,
        KEY_SPACE = 1ull << 9,
        KEY_SHIFT = 1ull << 10,
        KEY_CTRL = 1ull << 11,
        KEY_ESC = 1ull << 12,
        // Remaining letters
        KEY_R = 1ull << 13,
        KEY_F = 1ull << 14,
        KEY_G = 1ull << 15,
        KEY_H = 1ull << 16,
        KEY_I = 1ull << 17,
        KEY_J = 1ull << 18,
        KEY_K = 1ull << 19,
        KEY_L = 1ull << 20,
        KEY_Z = 1ull << 21,
        KEY_X = 1ull << 22,
        KEY_C = 1ull << 23,
        KEY_V = 1ull << 24,
        KEY_B = 1ull << 25,
        KEY_N = 1ull << 26,
        KEY_M = 1ull << 27,
        KEY_U = 1ull << 28,
        KEY_Y = 1ull << 29,
        KEY_T = 1ull << 30,
        KEY_O = 1ull << 31,
        KEY_P = 1ull << 32,
        // Number keys (top row)
        KEY_0 = 1ull << 33,
        KEY_1 = 1ull << 34,
        KEY_2 = 1ull << 35,
        KEY_3 = 1ull << 36,
        KEY_4 = 1ull << 37,
        KEY_5 = 1ull << 38,
        KEY_6 = 1ull << 39,
        KEY_7 = 1ull << 40,
        KEY_8 = 1ull << 41,
        KEY_9 = 1ull << 42,
        // Number keys (numpad)
        KEY_NUMPAD_0 = 1ull << 43,
        KEY_NUMPAD_1 = 1ull << 44,
        KEY_NUMPAD_2 = 1ull << 45,
        KEY_NUMPAD_3 = 1ull << 46,
        KEY_NUMPAD_4 = 1ull << 47,
        KEY_NUMPAD_5 = 1ull << 48,
        KEY_NUMPAD_6 = 1ull << 49,
        KEY_NUMPAD_7 = 1ull << 50,
        KEY_NUMPAD_8 = 1ull << 51,
        KEY_NUMPAD_9 = 1ull << 52
    };

    // Input events
    extern Event<InputState> onKey;
    extern Event<uint32_t, uint32_t, InputState> onMouseMove;
    extern Event<int32_t, InputState> onMouseScroll;

    // Test if a specific input key is pressed
    inline bool isPressed(InputState inputState, InputKeys key) 
    {
        return (inputState & key) != 0; 
    };

    void initInput(GLFWwindow* window);
    // Update input state based on GLFW window state
    void process(GLFWwindow* window);
}