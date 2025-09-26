#pragma once

#include <memory>

// Include imgui.h directly first to properly define ImGuiID
#include <imgui.h>

// Forward declarations for our app classes
class Window;
class Renderer;
class Scene;
class Camera;
class Editor;
class TransformGizmo;

// Forward declarations for ImGui docking branch
namespace ImGui {
    IMGUI_API ImGuiID DockBuilderAddNode(ImGuiID node_id, ImGuiDockNodeFlags flags = 0);
    IMGUI_API void DockBuilderRemoveNode(ImGuiID node_id);
    IMGUI_API void DockBuilderSetNodeSize(ImGuiID node_id, ImVec2 size);
    IMGUI_API ImGuiID DockBuilderSplitNode(ImGuiID node_id, ImGuiDir split_dir, float size_ratio_for_node_at_dir, ImGuiID* out_id_at_dir, ImGuiID* out_id_at_opposite_dir);
    IMGUI_API void DockBuilderDockWindow(const char* window_name, ImGuiID node_id);
    IMGUI_API void DockBuilderFinish(ImGuiID node_id);
}

// Needed for ImGui docking branch
#define ImGuiDockNodeFlags_DockSpace ImGuiDockNodeFlags_PassthruCentralNode

// Forward declarations for TransformGizmo enums
enum class GizmoMode;
enum class GizmoSpace;

// GUI components
class Viewport;
class SceneOutliner;
class DetailsPanel;
class ContentBrowser;
class ToolBar;
class StatusBar;
class LogWindow;

enum class ObjectType;

enum class LayoutConfig {
    UnrealStyle,    // Similar to Unreal Engine
    UnityStyle,     // Similar to Unity
    Minimal         // Focus on viewport
};

class GUI {
public:
    GUI(Window& window, Editor& editor);
    ~GUI();
    
    void update(Scene& scene, Camera& camera, Renderer& renderer);
    void render();
    void onWindowResize(int width, int height);
    
    bool isViewportFocused() const;
    bool isViewportHovered() const;
    
    // Layout management
    void setLayout(LayoutConfig config);

private:
    void newFrame();
    void setupDockspace();
    void setupLayout(ImGuiID dockspaceId);
    void showMenuBar();
    void showAboutDialog();
    void setupUnrealStyle();
    
    // Helper method to forward primitive creation to editor
    void createPrimitive(ObjectType type);
    
    Window& m_window;
    Editor& m_editor;
    
    // Layout configuration
    LayoutConfig m_layoutConfig;
    
    // UI components
    std::unique_ptr<Viewport> m_viewport;
    std::unique_ptr<SceneOutliner> m_sceneOutliner;
    std::unique_ptr<DetailsPanel> m_detailsPanel;
    std::unique_ptr<ContentBrowser> m_contentBrowser;
    std::unique_ptr<ToolBar> m_toolBar;
    std::unique_ptr<StatusBar> m_statusBar;
    std::unique_ptr<LogWindow> m_logWindow;
    
    // UI state
    bool m_showViewport = true;
    bool m_showOutliner = true;
    bool m_showDetails = true;
    bool m_showContentBrowser = true;
    bool m_showToolBar = true;
    bool m_showStatusBar = true;
    bool m_showLogger = true;
    bool m_showDemo = false;
    bool m_showAboutModal = false;
};