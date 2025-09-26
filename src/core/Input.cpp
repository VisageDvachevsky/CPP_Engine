#include "Input.h"
#include "Window.h"
#include "Logger.h"
#include <GLFW/glfw3.h>
#include <chrono>

Vec2 Input::s_mousePos{0, 0};
Vec2 Input::s_lastMousePos{0, 0};
Vec2 Input::s_mouseDelta{0, 0};
float Input::s_scrollDelta = 0.0f;
bool Input::s_firstMouse = true;
GLFWwindow* Input::s_windowHandle = nullptr;

std::unordered_map<int, bool> Input::s_keyPressed;
std::unordered_map<int, bool> Input::s_keyHeld;
std::unordered_map<int, bool> Input::s_mouseButtonPressed;
std::unordered_map<int, bool> Input::s_mouseButtonReleased;
std::unordered_map<int, bool> Input::s_lastKeyPressed;
std::unordered_map<int, bool> Input::s_lastMouseButtonPressed;
std::unordered_map<int, std::chrono::steady_clock::time_point> Input::s_lastClickTime;
std::unordered_map<int, bool> Input::s_mouseButtonDoubleClicked;

// GLFW callbacks
static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    Input::s_scrollDelta = static_cast<float>(yoffset);
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        Input::s_keyPressed[key] = true;
        Input::s_keyHeld[key] = true;
    } 
    else if (action == GLFW_RELEASE) {
        Input::s_keyHeld[key] = false;
        Input::s_keyPressed[key] = false;
    }
}

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    using namespace std::chrono;
    
    if (action == GLFW_PRESS) {
        // Check for double click before setting pressed = true
        auto now = steady_clock::now();
        if (Input::s_lastClickTime.find(button) != Input::s_lastClickTime.end()) {
            auto elapsed = duration_cast<milliseconds>(now - Input::s_lastClickTime[button]).count() / 1000.0;
            
            if (elapsed < Input::DOUBLE_CLICK_TIME) {
                Input::s_mouseButtonDoubleClicked[button] = true;
                LOG_DEBUG("Double-click detected for button {}, elapsed: {:.3f}s", 
                         button, elapsed);
            }
        }
        
        Input::s_lastClickTime[button] = now;
        Input::s_mouseButtonPressed[button] = true;
    } 
    else if (action == GLFW_RELEASE) {
        Input::s_mouseButtonPressed[button] = false;
        Input::s_mouseButtonReleased[button] = true;
    }
}

static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    Input::s_mousePos = Vec2{static_cast<float>(xpos), static_cast<float>(ypos)};
}

void Input::init(GLFWwindow* window) {
    s_windowHandle = window;
    
    // Set up callbacks
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    
    // Initialize initial mouse position and time
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    s_mousePos = Vec2{static_cast<float>(x), static_cast<float>(y)};
    s_lastMousePos = s_mousePos;
    
    LOG_INFO("Input system initialized with double-click time of {:.3f} seconds", DOUBLE_CLICK_TIME);
}

void Input::update() {
    if (!s_windowHandle) {
        LOG_ERROR("Input::update() called before initialization");
        return;
    }
    
    if (s_firstMouse) {
        s_lastMousePos = s_mousePos;
        s_firstMouse = false;
        s_mouseDelta = Vec2{0, 0};
    } else {
        s_mouseDelta = s_mousePos - s_lastMousePos;
        s_lastMousePos = s_mousePos;
    }
    
    // Reset double-click flags on the next frame after detection
    for (auto& [button, isDoubleClicked] : s_mouseButtonDoubleClicked) {
        if (isDoubleClicked) {
            LOG_DEBUG("Resetting double click flag for button {}", button);
            isDoubleClicked = false;
        }
    }
    
    // Reset single press flags
    for (auto& [key, isPressed] : s_keyPressed) {
        if (isPressed && s_lastKeyPressed[key]) {
            isPressed = false;
        }
        s_lastKeyPressed[key] = s_keyHeld[key];
    }
    
    // Reset mouse button release flags
    for (auto& [button, isReleased] : s_mouseButtonReleased) {
        isReleased = false;
    }
    
    // Store current mouse button states
    for (auto& [button, isPressed] : s_mouseButtonPressed) {
        s_lastMouseButtonPressed[button] = isPressed;
    }
}

bool Input::isKeyPressed(int key) {
    return s_keyPressed[key];
}

bool Input::isKeyHeld(int key) {
    return s_keyHeld[key];
}

bool Input::isMouseButtonPressed(int button) {
    return s_mouseButtonPressed[button];
}

bool Input::isMouseButtonReleased(int button) {
    return s_mouseButtonReleased[button];
}

Vec2 Input::getMousePosition() {
    return s_mousePos;
}

Vec2 Input::getMouseDelta() {
    return s_mouseDelta;
}

bool Input::isMouseButtonDoubleClicked(int button) {
    return s_mouseButtonDoubleClicked[button];
}

float Input::getScrollDelta() {
    float delta = s_scrollDelta;
    if (delta != 0.0f) {
        LOG_DEBUG("Reading scroll delta: {}", delta);
    }
    s_scrollDelta = 0.0f; 
    return delta;
}

void Input::setMouseCursorEnabled(bool enabled) {
    if (!s_windowHandle) return;
    glfwSetInputMode(s_windowHandle, GLFW_CURSOR, enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}