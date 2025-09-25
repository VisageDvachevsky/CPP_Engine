#pragma once

#include <memory>

class Window;
class Renderer;
class Scene;
class Editor;
class Camera;
class FileWatcher;

class Application {
public:
    Application();
    ~Application();
    
    void run();
    
    static Application* getInstance() { return s_instance; }

private:
    void init();
    void update(float dt);
    void render();
    void cleanup();
    void handleEvents();
    
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<Scene> m_scene;
    std::unique_ptr<Editor> m_editor;
    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<FileWatcher> m_fileWatcher;
    
    bool m_running = true;
    float m_lastFrameTime = 0.0f;
    
    static Application* s_instance;
};