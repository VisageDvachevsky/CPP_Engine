#pragma once

#include "math/Vec2.h"

struct GLFWwindow;
class Window;

class Input {
public:
    static void update(const Window& window);
    
    static bool isKeyPressed(int key);
    static bool isKeyHeld(int key);
    static bool isMouseButtonPressed(int button);
    static bool isMouseButtonReleased(int button);
    
    static Vec2 getMousePosition();
    static Vec2 getMouseDelta();
    static float getScrollDelta();
    
    static void setMouseCursorEnabled(bool enabled);

    // For internal use
    static float s_scrollDelta;

private:
    static Vec2 s_mousePos;
    static Vec2 s_lastMousePos;
    static Vec2 s_mouseDelta;
    static bool s_firstMouse;
    static GLFWwindow* s_windowHandle;
};