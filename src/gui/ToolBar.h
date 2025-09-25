#pragma once

class Editor;
class Scene;
class Camera;
class Renderer;

class ToolBar {
public:
    ToolBar(Editor& editor);
    ~ToolBar() = default;
    
    void show(Scene& scene, Camera& camera, Renderer& renderer);

private:
    void showTransformTools();
    void showRenderSettings(Renderer& renderer);
    void showCameraControls(Camera& camera);
    
    Editor& m_editor;
};