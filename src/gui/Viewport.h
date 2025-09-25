#pragma once

#include "math/Vec2.h"
#include <memory>

class Editor;
class Scene;
class Camera;
class Renderer;
class Framebuffer;

class Viewport {
public:
    Viewport(Editor& editor);
    ~Viewport();
    
    void show(Scene& scene, Camera& camera, Renderer& renderer);
    void onResize(int width, int height);
    
    bool isFocused() const { return m_isFocused; }
    bool isHovered() const { return m_isHovered; }
    
    Vec2 getSize() const { return m_viewportSize; }

private:
    void handleInput(Scene& scene, Camera& camera);
    void renderViewportContent(Scene& scene, Camera& camera, Renderer& renderer);
    void renderOverlays(Scene& scene, Camera& camera, Renderer& renderer);
    void renderGizmos(Scene& scene, Camera& camera);
    
    Editor& m_editor;
    std::unique_ptr<Framebuffer> m_framebuffer;
    
    Vec2 m_viewportSize{1280, 720};
    Vec2 m_viewportPos{0, 0};
    
    bool m_isFocused = false;
    bool m_isHovered = false;
    bool m_firstFrame = true;
};