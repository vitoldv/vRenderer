#include "input_handler.h"

namespace inp
{
    Event<InputState> onKey;
    Event<uint32_t, uint32_t, InputState> onMouseMove;
    Event<int32_t, InputState> onMouseScroll;

    // Current keyboard input state
    InputState inputState;

    void invokeEvents(GLFWwindow* window)
    {
        //  KEYBOARD
        if (inputState != 0)
        {
            onKey.invoke(inputState);
        }

        // MOUSE MOVE
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        onMouseMove.invoke(xpos, ypos, inputState);
    }

    void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
    {
        onMouseScroll.invoke(yoffset, inputState);
    }

    void initInput(GLFWwindow* window)
    {
        glfwSetScrollCallback(window, scroll_callback);
    }

    // Update input state based on GLFW window state
    void process(GLFWwindow* window)
    {
        // Reset flags
        inputState = NONE;

        // Update mouse buttons
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
            inputState |= MOUSE_LEFT;
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
            inputState |= MOUSE_RIGHT;
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
            inputState |= MOUSE_MIDDLE;

        // Update keyboard keys
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            inputState |= KEY_W;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            inputState |= KEY_A;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            inputState |= KEY_S;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            inputState |= KEY_D;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            inputState |= KEY_Q;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            inputState |= KEY_E;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            inputState |= KEY_SPACE;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS)
            inputState |= KEY_SHIFT;
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)
            inputState |= KEY_CTRL;
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            inputState |= KEY_ESC;

        // Added remaining letter keys
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
            inputState |= KEY_R;
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
            inputState |= KEY_F;
        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
            inputState |= KEY_G;
        if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
            inputState |= KEY_H;
        if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
            inputState |= KEY_I;
        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
            inputState |= KEY_J;
        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
            inputState |= KEY_K;
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
            inputState |= KEY_L;
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
            inputState |= KEY_Z;
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
            inputState |= KEY_X;
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
            inputState |= KEY_C;
        if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
            inputState |= KEY_V;
        if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
            inputState |= KEY_B;
        if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
            inputState |= KEY_N;
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
            inputState |= KEY_M;
        if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
            inputState |= KEY_U;
        if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
            inputState |= KEY_Y;
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
            inputState |= KEY_T;
        if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
            inputState |= KEY_O;
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
            inputState |= KEY_P;

        // Added number keys (top row)
        if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
            inputState |= KEY_0;
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
            inputState |= KEY_1;
        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
            inputState |= KEY_2;
        if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
            inputState |= KEY_3;
        if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
            inputState |= KEY_4;
        if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
            inputState |= KEY_5;
        if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
            inputState |= KEY_6;
        if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS)
            inputState |= KEY_7;
        if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS)
            inputState |= KEY_8;
        if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS)
            inputState |= KEY_9;

        // Added numpad keys
        if (glfwGetKey(window, GLFW_KEY_KP_0) == GLFW_PRESS)
            inputState |= KEY_NUMPAD_0;
        if (glfwGetKey(window, GLFW_KEY_KP_1) == GLFW_PRESS)
            inputState |= KEY_NUMPAD_1;
        if (glfwGetKey(window, GLFW_KEY_KP_2) == GLFW_PRESS)
            inputState |= KEY_NUMPAD_2;
        if (glfwGetKey(window, GLFW_KEY_KP_3) == GLFW_PRESS)
            inputState |= KEY_NUMPAD_3;
        if (glfwGetKey(window, GLFW_KEY_KP_4) == GLFW_PRESS)
            inputState |= KEY_NUMPAD_4;
        if (glfwGetKey(window, GLFW_KEY_KP_5) == GLFW_PRESS)
            inputState |= KEY_NUMPAD_5;
        if (glfwGetKey(window, GLFW_KEY_KP_6) == GLFW_PRESS)
            inputState |= KEY_NUMPAD_6;
        if (glfwGetKey(window, GLFW_KEY_KP_7) == GLFW_PRESS)
            inputState |= KEY_NUMPAD_7;
        if (glfwGetKey(window, GLFW_KEY_KP_8) == GLFW_PRESS)
            inputState |= KEY_NUMPAD_8;
        if (glfwGetKey(window, GLFW_KEY_KP_9) == GLFW_PRESS)
            inputState |= KEY_NUMPAD_9;

        invokeEvents(window);
    }
}