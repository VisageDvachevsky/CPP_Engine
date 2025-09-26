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

// Коллбэки GLFW
static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    Input::s_scrollDelta = static_cast<float>(yoffset);
}

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        Input::s_keyPressed[key] = true;
        Input::s_keyHeld[key] = true;    } 
    else if (action == GLFW_RELEASE) {
        Input::s_keyHeld[key] = false;
        Input::s_keyPressed[key] = false;
    }
}

static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    using namespace std::chrono;
    
    if (action == GLFW_PRESS) {
        // Сначала проверяем двойной клик перед установкой pressed = true
        auto now = steady_clock::now();
        if (Input::s_lastClickTime.find(button) != Input::s_lastClickTime.end()) {
            auto elapsed = duration_cast<duration<double>>(now - Input::s_lastClickTime[button]).count();
            
            if (elapsed < Input::DOUBLE_CLICK_TIME) {
                Input::s_mouseButtonDoubleClicked[button] = true;
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
    
    // Устанавливаем коллбэки
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    
    // Инициализация начальной позиции мыши и времени
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
    
    // Сбрасываем флаги двойных кликов на следующем кадре после обнаружения
    for (auto& pair : s_mouseButtonDoubleClicked) {
        if (pair.second) {
            LOG_DEBUG("Resetting double click flag for button {}", pair.first);
            pair.second = false;
        }
    }
    
    // Сбрасываем флаги одиночных нажатий
    for (auto& pair : s_keyPressed) {
        if (pair.second && s_lastKeyPressed[pair.first]) {
            pair.second = false;
        }
        s_lastKeyPressed[pair.first] = s_keyHeld[pair.first];
    }
    
    // Сбрасываем флаги отпускания кнопок мыши
    for (auto& pair : s_mouseButtonReleased) {
        pair.second = false;
    }
    
    // Сохраняем текущее состояние кнопок мыши
    for (auto& pair : s_mouseButtonPressed) {
        s_lastMouseButtonPressed[pair.first] = pair.second;
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
    s_scrollDelta = 0.0f; // Сбрасываем после чтения
    return delta;
}

void Input::setMouseCursorEnabled(bool enabled) {
    if (!s_windowHandle) return;
    glfwSetInputMode(s_windowHandle, GLFW_CURSOR, enabled ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}