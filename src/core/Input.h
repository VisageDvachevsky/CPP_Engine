#pragma once

#include "math/Vec2.h"
#include <unordered_map>
#include <chrono>

struct GLFWwindow;

class Input {
public:
    static void init(GLFWwindow* window);
    static void update();
    
    static bool isKeyPressed(int key);
    static bool isKeyHeld(int key);
    static bool isMouseButtonPressed(int button);
    static bool isMouseButtonReleased(int button);
    static bool isMouseButtonDoubleClicked(int button); 
    
    static Vec2 getMousePosition();
    static Vec2 getMouseDelta();
    static float getScrollDelta();
    
    static void setMouseCursorEnabled(bool enabled);

    static Vec2 s_mousePos;
    static Vec2 s_lastMousePos;
    static Vec2 s_mouseDelta;
    static float s_scrollDelta;
    static bool s_firstMouse;
    static GLFWwindow* s_windowHandle;
    
    static std::unordered_map<int, bool> s_keyPressed;
    static std::unordered_map<int, bool> s_keyHeld;
    static std::unordered_map<int, bool> s_mouseButtonPressed;
    static std::unordered_map<int, bool> s_mouseButtonReleased;
    static std::unordered_map<int, bool> s_lastKeyPressed;
    static std::unordered_map<int, bool> s_lastMouseButtonPressed;
    
    static std::unordered_map<int, std::chrono::steady_clock::time_point> s_lastClickTime;
    static std::unordered_map<int, bool> s_mouseButtonDoubleClicked;
    static constexpr double DOUBLE_CLICK_TIME = 0.3; 
};