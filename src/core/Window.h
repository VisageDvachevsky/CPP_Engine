#pragma once

#include <string>
#include <functional>

struct GLFWwindow;

class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();
    
    bool shouldClose() const;
    void pollEvents();
    void swapBuffers();
    
    GLFWwindow* handle() const { return m_window; }
    
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    float getAspectRatio() const { return static_cast<float>(m_width) / m_height; }
    
    void setResizeCallback(std::function<void(int, int)> callback) { m_resizeCallback = callback; }

private:
    void setupCallbacks();
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    
    GLFWwindow* m_window = nullptr;
    int m_width, m_height;
    std::string m_title;
    std::function<void(int, int)> m_resizeCallback;
};