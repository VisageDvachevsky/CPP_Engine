#pragma once

#include <memory>

class Window;
class Renderer;
class Scene;
class Camera;
class EditorCamera;
class SelectionManager;
class TransformGizmo;
class GUI;

enum class EditorMode {
    Object,
    Edit,
    Play
};

class Editor {
public:
    Editor(Window& window, Renderer& renderer, Scene& scene, Camera& camera);
    ~Editor();
    
    void update(float dt);
    void render();
    
    void onWindowResize(int width, int height);
    
    EditorMode getMode() const { return m_mode; }
    void setMode(EditorMode mode) { m_mode = mode; }
    
    SelectionManager& getSelection() { return *m_selectionManager; }
    TransformGizmo& getGizmo() { return *m_transformGizmo; }

private:
    void initializeEditor();
    void updateEditor(float dt);
    void renderEditor();
    
    Window& m_window;
    Renderer& m_renderer;
    Scene& m_scene;
    Camera& m_camera;
    
    std::unique_ptr<EditorCamera> m_editorCamera;
    std::unique_ptr<SelectionManager> m_selectionManager;
    std::unique_ptr<TransformGizmo> m_transformGizmo;
    std::unique_ptr<GUI> m_gui;
    
    EditorMode m_mode = EditorMode::Object;
    bool m_isViewportFocused = false;
    bool m_isViewportHovered = false;
};