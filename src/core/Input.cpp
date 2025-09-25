#include "Input.h"
#include "Window.h"
#include <GLFW/glfw3.h>

Vec2 Input::s_mousePos{0, 0};
Vec2 Input::s_lastMousePos{0, 0};
Vec2 Input::s_mouseDelta{0, 0};
float Input::s_scrollDelta = 0.0f;
bool Input::s_firstMouse = true;
GLFWwindow* Input::s_windowHandle = nullptr;

static void scrollCallback(GLFWwindow*, double, double yoffset) {
    Input::s_scrollDelta = static_cast<float>(yoffset);
}

void Input::update(const Window& window) {
    s_windowHandle = window.handle();
    
    // Mouse position
    double x, y;
    glfwGetCursorPos(s_windowHandle, &x, &y);
    s_mousePos = Vec2{static_cast<float>(x), static_cast<float>(y)};
    
    if (s_firstMouse) {
        s_lastMousePos = s_mousePos;
        s_firstMouse = false;
    }
    
    s_mouseDelta = s_mousePos - s_lastMousePos;
    s_lastMousePos = s_mousePos;
    
    // Setup scroll callback once
    static bool callbackSet = false;
    if (!callbackSet) {
        glfwSetScrollCallback(s_windowHandle, scrollCallback);
        callbackSet = true;
    }
}

bool Input::isKeyPressed(int key) {
    if (!s_windowHandle) return false;
    return glfwGetKey(s_windowHandle, key) == GLFW_PRESS;
}

bool Input::isKeyHeld(int key) {
    if (!s_windowHandle) return false;
    return glfwGetKey(s_windowHandle, key) == GLFW_PRESS || glfwGetKey(s_windowHandle, key) == GLFW_REPEAT;
}

bool Input::isMouseButtonPressed(int button) {
    if (!s_windowHandle) return false;
    return glfwGetMouseButton(s_windowHandle, button) == GLFW_PRESS;
}

bool Input::isMouseButtonReleased(int button) {
    if (!s_windowHandle) return false;
    return glfwGetMouseButton(s_windowHandle, button) == GLFW_RELEASE;
}

Vec2 Input::getMousePosition() {
    return s_mousePos;
}

Vec2 Input::getMouseDelta() {
    return s_mouseDelta;
}

float Input::getScrollDelta() {
    float delta = s_scrollDelta;
    s_scrollDelta = 0.0f; // Reset after reading
    return delta;
}

void Input::setMouseCursorEnabled(bool enabled) {
    if (!s_windowHandle) return;
    glfwSetInputMode(s_windowHandle, GLFW_CURSOR, enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}