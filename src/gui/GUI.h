#pragma once

#include <memory>

class Window;
class Scene;
class Camera;
class Renderer;
class Editor;
class Viewport;
class SceneOutliner;
class DetailsPanel;
class ContentBrowser;
class ToolBar;
class StatusBar;
class LogWindow;

class GUI {
public:
    GUI(Window& window, Editor& editor);
    ~GUI();
    
    void update(Scene& scene, Camera& camera, Renderer& renderer);
    void render();
    void onWindowResize(int width, int height);
    
    bool isViewportFocused() const;
    bool isViewportHovered() const;

private:
    void newFrame();
    void setupDockspace();
    void setupUnrealStyle();
    void showMenuBar();
    
    Window& m_window;
    Editor& m_editor;
    
    std::unique_ptr<Viewport> m_viewport;
    std::unique_ptr<SceneOutliner> m_sceneOutliner;
    std::unique_ptr<DetailsPanel> m_detailsPanel;
    std::unique_ptr<ContentBrowser> m_contentBrowser;
    std::unique_ptr<ToolBar> m_toolBar;
    std::unique_ptr<StatusBar> m_statusBar;
    std::unique_ptr<LogWindow> m_logWindow;
    
    bool m_showViewport = true;
    bool m_showOutliner = true;
    bool m_showDetails = true;
    bool m_showContentBrowser = true;
    bool m_showToolBar = true;
    bool m_showStatusBar = true;
    bool m_showLogger = true;
    bool m_showDemo = false;
};