#include "GUI.h"
#include "Viewport.h"
#include "SceneOutliner.h"
#include "DetailsPanel.h"
#include "ContentBrowser.h"
#include "ToolBar.h"
#include "StatusBar.h"
#include "LogWindow.h"
#include "core/Window.h"
#include "core/Logger.h"  
#include "editor/Editor.h"
#include "editor/TransformGizmo.h" // Include TransformGizmo directly for access to enums
#include "scene/Scene.h"
#include "scene/Camera.h"
#include "scene/Object.h"
#include "renderer/Renderer.h"

// ImGui includes
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

GUI::GUI(Window& window, Editor& editor) : m_window(window), m_editor(editor) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    
    // Load font - commented out until you have this asset
    // io.Fonts->AddFontFromFileTTF("assets/fonts/Roboto-Regular.ttf", 16.0f);
    
    setupUnrealStyle();
    
    ImGui_ImplGlfw_InitForOpenGL(window.handle(), true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    
    m_viewport = std::make_unique<Viewport>(m_editor);
    m_sceneOutliner = std::make_unique<SceneOutliner>();
    m_detailsPanel = std::make_unique<DetailsPanel>();
    m_contentBrowser = std::make_unique<ContentBrowser>();
    m_toolBar = std::make_unique<ToolBar>(m_editor);
    m_statusBar = std::make_unique<StatusBar>();
    m_logWindow = std::make_unique<LogWindow>();
    
    // Default layout
    m_layoutConfig = LayoutConfig::UnrealStyle;
    
    LOG_INFO("GUI initialized with Unreal-style layout");
}

GUI::~GUI() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void GUI::newFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GUI::update(Scene& scene, Camera& camera, Renderer& renderer) {
    newFrame();
    
    setupDockspace();
    showMenuBar();
    
    // Toolbar is always at the top
    if (m_showToolBar) {
        m_toolBar->show(scene, camera, renderer);
    }
    
    // Main panels
    if (m_showViewport) m_viewport->show(scene, camera, renderer);
    if (m_showOutliner) m_sceneOutliner->show(scene);
    if (m_showDetails) m_detailsPanel->show(scene);
    if (m_showContentBrowser) m_contentBrowser->show();
    if (m_showLogger) m_logWindow->show();
    
    // Status bar is always at the bottom
    if (m_showStatusBar) {
        m_statusBar->show(renderer);
    }
    
    // Demo window if enabled
    if (m_showDemo) {
        ImGui::ShowDemoWindow(&m_showDemo);
    }
    
    // About modal dialog
    if (m_showAboutModal) {
        showAboutDialog();
    }
}

void GUI::render() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

void GUI::onWindowResize(int width, int height) {
    if (m_viewport) {
        m_viewport->onResize(width, height);
    }
}

bool GUI::isViewportFocused() const {
    return m_viewport ? m_viewport->isFocused() : false;
}

bool GUI::isViewportHovered() const {
    return m_viewport ? m_viewport->isHovered() : false;
}

void GUI::setupDockspace() {
    static bool opt_fullscreen = true;
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    if (opt_fullscreen) {
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | 
                       ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    }
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar();
    
    if (opt_fullscreen)
        ImGui::PopStyleVar(2);
    
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
        ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
        
        // First-time docking setup
        static bool first_time = true;
        if (first_time) {
            first_time = false;
            ImGui::DockBuilderRemoveNode(dockspace_id);
            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id, io.DisplaySize);
            
            // Create dock layout based on current config
            setupLayout(dockspace_id);
            
            ImGui::DockBuilderFinish(dockspace_id);
        }
        
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
    }
    
    ImGui::End();
}

void GUI::setupLayout(ImGuiID dockspaceId) {
    // Setup node IDs
    ImGuiID dock_main = dockspaceId;
    ImGuiID dock_left = 0, dock_right = 0, dock_bottom = 0, dock_bottom_right = 0;
    
    switch (m_layoutConfig) {
        case LayoutConfig::UnrealStyle: {
            // Split the dockspace into sections
            ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Left, 0.2f, &dock_left, &dock_main);
            ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Right, 0.75f, &dock_main, &dock_right);
            ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Down, 0.25f, &dock_main, &dock_bottom);
            ImGui::DockBuilderSplitNode(dock_bottom, ImGuiDir_Right, 0.4f, &dock_bottom, &dock_bottom_right);
            
            // Dock windows
            ImGui::DockBuilderDockWindow("Viewport", dock_right);
            ImGui::DockBuilderDockWindow("Scene Outliner", dock_left);
            ImGui::DockBuilderDockWindow("Details", dock_main);
            ImGui::DockBuilderDockWindow("Content Browser", dock_bottom);
            ImGui::DockBuilderDockWindow("Logger", dock_bottom_right);
            break;
        }
        
        case LayoutConfig::UnityStyle: {
            // Split the dockspace into sections
            ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Right, 0.25f, &dock_right, &dock_main);
            ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Down, 0.25f, &dock_main, &dock_bottom);
            ImGui::DockBuilderSplitNode(dock_bottom, ImGuiDir_Right, 0.6f, &dock_bottom, &dock_bottom_right);
            
            // Dock windows
            ImGui::DockBuilderDockWindow("Viewport", dock_main);
            ImGui::DockBuilderDockWindow("Scene Outliner", dock_main);
            ImGui::DockBuilderDockWindow("Details", dock_right);
            ImGui::DockBuilderDockWindow("Content Browser", dock_bottom);
            ImGui::DockBuilderDockWindow("Logger", dock_bottom_right);
            break;
        }
        
        case LayoutConfig::Minimal: {
            // Split the dockspace into sections
            ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Down, 0.2f, &dock_bottom, &dock_main);
            ImGui::DockBuilderSplitNode(dock_bottom, ImGuiDir_Left, 0.33f, &dock_left, &dock_bottom);
            ImGui::DockBuilderSplitNode(dock_bottom, ImGuiDir_Left, 0.5f, &dock_main, &dock_right);
            
            // Dock windows
            ImGui::DockBuilderDockWindow("Viewport", dock_main);
            ImGui::DockBuilderDockWindow("Scene Outliner", dock_left);
            ImGui::DockBuilderDockWindow("Details", dock_bottom);
            ImGui::DockBuilderDockWindow("Logger", dock_right);
            ImGui::DockBuilderDockWindow("Content Browser", dock_left);
            break;
        }
    }
}

void GUI::setLayout(LayoutConfig config) {
    m_layoutConfig = config;
    
    // Force re-initialization of docking layout
    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetIO().DisplaySize);
    
    setupLayout(dockspace_id);
    
    ImGui::DockBuilderFinish(dockspace_id);
    
    LOG_INFO("Layout changed to {}", 
        config == LayoutConfig::UnrealStyle ? "Unreal Style" :
        config == LayoutConfig::UnityStyle ? "Unity Style" : "Minimal");
}

void GUI::showMenuBar() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene", "Ctrl+N")) {
                // TODO: New scene
            }
            if (ImGui::MenuItem("Open Scene...", "Ctrl+O")) {
                // TODO: Open scene
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
                // TODO: Save scene
            }
            if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S")) {
                // TODO: Save scene as
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Alt+F4")) {
                // Instead of m_window.close() which doesn't exist,
                // we set the window should close flag using GLFW
                glfwSetWindowShouldClose(m_window.handle(), GLFW_TRUE);
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {
                // TODO: Undo system
            }
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) {
                // TODO: Redo system
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Delete Selection", "Delete")) {
                // TODO: Delete selected objects
            }
            if (ImGui::MenuItem("Duplicate", "Ctrl+D")) {
                // TODO: Duplicate selected objects
            }
            if (ImGui::MenuItem("Frame Selected", "F")) {
                // TODO: Frame selected object
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Project Settings...")) {
                // TODO: Project settings
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("View")) {
            if (ImGui::BeginMenu("Panels")) {
                ImGui::MenuItem("Viewport", "Alt+1", &m_showViewport);
                ImGui::MenuItem("Scene Outliner", "Alt+2", &m_showOutliner);
                ImGui::MenuItem("Details", "Alt+3", &m_showDetails);
                ImGui::MenuItem("Content Browser", "Alt+4", &m_showContentBrowser);
                ImGui::MenuItem("Logger", "Alt+5", &m_showLogger);
                ImGui::MenuItem("Tool Bar", nullptr, &m_showToolBar);
                ImGui::MenuItem("Status Bar", nullptr, &m_showStatusBar);
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Layout")) {
                bool unrealStyle = (m_layoutConfig == LayoutConfig::UnrealStyle);
                bool unityStyle = (m_layoutConfig == LayoutConfig::UnityStyle);
                bool minimal = (m_layoutConfig == LayoutConfig::Minimal);
                
                if (ImGui::MenuItem("Unreal Style", nullptr, &unrealStyle)) {
                    setLayout(LayoutConfig::UnrealStyle);
                }
                if (ImGui::MenuItem("Unity Style", nullptr, &unityStyle)) {
                    setLayout(LayoutConfig::UnityStyle);
                }
                if (ImGui::MenuItem("Minimal", nullptr, &minimal)) {
                    setLayout(LayoutConfig::Minimal);
                }
                
                ImGui::EndMenu();
            }
            
            ImGui::Separator();
            
            if (ImGui::BeginMenu("Camera")) {
                if (ImGui::MenuItem("Top View", "Num7")) {
                    // TODO: Set camera to top view
                }
                if (ImGui::MenuItem("Front View", "Num1")) {
                    // TODO: Set camera to front view
                }
                if (ImGui::MenuItem("Side View", "Num3")) {
                    // TODO: Set camera to side view
                }
                if (ImGui::MenuItem("Perspective", "Num5")) {
                    // TODO: Toggle perspective/orthographic
                }
                ImGui::EndMenu();
            }
            
            ImGui::Separator();
            
            if (ImGui::MenuItem("ImGui Demo", nullptr, &m_showDemo)) {
                // Toggle ImGui demo window
            }
            
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Reload Shaders", "F5")) {
                m_editor.getRenderer().reloadShaders();
            }
            
            ImGui::Separator();
            
            // Transform tools
            bool isObjectMode = (m_editor.getMode() == EditorMode::Object);
            bool isEditMode = (m_editor.getMode() == EditorMode::Edit);
            bool isPlayMode = (m_editor.getMode() == EditorMode::Play);
            
            if (ImGui::MenuItem("Object Mode", "1", &isObjectMode)) {
                m_editor.setMode(EditorMode::Object);
            }
            if (ImGui::MenuItem("Edit Mode", "2", &isEditMode)) {
                m_editor.setMode(EditorMode::Edit);
            }
            if (ImGui::MenuItem("Play Mode", "3", &isPlayMode)) {
                m_editor.setMode(EditorMode::Play);
            }
            
            ImGui::Separator();
            
            // Transform gizmo tools - using editor's gizmo rather than direct access
            TransformGizmo& gizmo = m_editor.getGizmo();
            
            bool isTranslate = (gizmo.getMode() == GizmoMode::Translate);
            bool isRotate = (gizmo.getMode() == GizmoMode::Rotate);
            bool isScale = (gizmo.getMode() == GizmoMode::Scale);
            
            if (ImGui::MenuItem("Translate", "W", &isTranslate)) {
                gizmo.setMode(GizmoMode::Translate);
            }
            if (ImGui::MenuItem("Rotate", "E", &isRotate)) {
                gizmo.setMode(GizmoMode::Rotate);
            }
            if (ImGui::MenuItem("Scale", "R", &isScale)) {
                gizmo.setMode(GizmoMode::Scale);
            }
            
            ImGui::Separator();
            
            bool isWorld = (gizmo.getSpace() == GizmoSpace::World);
            bool isLocal = (gizmo.getSpace() == GizmoSpace::Local);
            
            if (ImGui::MenuItem("World Space", "T", &isWorld)) {
                gizmo.setSpace(GizmoSpace::World);
            }
            if (ImGui::MenuItem("Local Space", "Y", &isLocal)) {
                gizmo.setSpace(GizmoSpace::Local);
            }
            
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Create")) {
            if (ImGui::BeginMenu("Primitives")) {
                if (ImGui::MenuItem("Sphere")) {
                    createPrimitive(ObjectType::Sphere);
                }
                if (ImGui::MenuItem("Cube")) {
                    createPrimitive(ObjectType::Cube);
                }
                if (ImGui::MenuItem("Plane")) {
                    createPrimitive(ObjectType::Plane);
                }
                ImGui::EndMenu();
            }
            
            if (ImGui::BeginMenu("Lights")) {
                if (ImGui::MenuItem("Point Light")) {
                    // TODO: Create point light
                }
                if (ImGui::MenuItem("Directional Light")) {
                    // TODO: Create directional light
                }
                ImGui::EndMenu();
            }
            
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                m_showAboutModal = true;
            }
            ImGui::EndMenu();
        }
        
        // Mode switcher in the menubar (right-aligned)
        ImGui::PushStyleColor(ImGuiCol_Button, m_editor.getMode() == EditorMode::Object ? 
                             ImVec4{0.3f, 0.5f, 0.7f, 1.0f} : ImVec4{0.2f, 0.2f, 0.2f, 1.0f});
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImGui::SameLine(windowSize.x - 300);
        if (ImGui::Button("Object", ImVec2(80, 0))) {
            m_editor.setMode(EditorMode::Object);
        }
        ImGui::PopStyleColor();
        
        ImGui::PushStyleColor(ImGuiCol_Button, m_editor.getMode() == EditorMode::Edit ? 
                             ImVec4{0.3f, 0.5f, 0.7f, 1.0f} : ImVec4{0.2f, 0.2f, 0.2f, 1.0f});
        ImGui::SameLine();
        if (ImGui::Button("Edit", ImVec2(80, 0))) {
            m_editor.setMode(EditorMode::Edit);
        }
        ImGui::PopStyleColor();
        
        ImGui::PushStyleColor(ImGuiCol_Button, m_editor.getMode() == EditorMode::Play ? 
                             ImVec4{0.3f, 0.7f, 0.3f, 1.0f} : ImVec4{0.2f, 0.2f, 0.2f, 1.0f});
        ImGui::SameLine();
        if (ImGui::Button("Play", ImVec2(80, 0))) {
            m_editor.setMode(EditorMode::Play);
        }
        ImGui::PopStyleColor();
        
        ImGui::EndMainMenuBar();
    }
}

void GUI::showAboutDialog() {
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    
    if (ImGui::BeginPopupModal("About MiniGPU Engine", &m_showAboutModal, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("MiniGPU Engine v1.0");
        ImGui::Separator();
        
        ImGui::Text("A modern OpenGL-based renderer and game engine");
        ImGui::Text("with real-time path tracing capabilities.");
        ImGui::Spacing();
        
        ImGui::Text("Built with:");
        ImGui::BulletText("OpenGL 3.3");
        ImGui::BulletText("GLFW for window management");
        ImGui::BulletText("ImGui for the editor interface");
        ImGui::BulletText("ImGuizmo for transform controls");
        ImGui::BulletText("stb_image for texture loading");
        ImGui::Spacing();
        
        ImGui::Separator();
        
        if (ImGui::Button("Close", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
            m_showAboutModal = false;
        }
        
        ImGui::EndPopup();
    } else {
        ImGui::OpenPopup("About MiniGPU Engine");
    }
}

void GUI::createPrimitive(ObjectType type) {
    // Forward to the editor's creation logic
    m_editor.createPrimitive(type);
}

void GUI::setupUnrealStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Unreal-inspired dark theme
    style.WindowPadding = ImVec2(8.0f, 8.0f);
    style.FramePadding = ImVec2(5.0f, 3.0f);
    style.ItemSpacing = ImVec2(6.0f, 6.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
    style.TouchExtraPadding = ImVec2(0.0f, 0.0f);
    style.IndentSpacing = 20.0f;
    style.ScrollbarSize = 14.0f;
    style.GrabMinSize = 10.0f;
    
    // Rounding
    style.WindowRounding = 4.0f;
    style.ChildRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 4.0f;
    style.ScrollbarRounding = 9.0f;
    style.GrabRounding = 3.0f;
    style.TabRounding = 4.0f;
    
    // Alignment
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    
    // Colors
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_Text]                   = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_PopupBg]                = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_Border]                 = ImVec4(0.30f, 0.30f, 0.30f, 0.50f);
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_MenuBarBg]              = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.90f, 0.90f, 0.90f, 0.50f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(1.00f, 0.50f, 0.00f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(1.00f, 0.70f, 0.10f, 1.00f);
    colors[ImGuiCol_Button]                 = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_Header]                 = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    colors[ImGuiCol_Separator]              = ImVec4(0.30f, 0.30f, 0.30f, 0.50f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.40f, 0.40f, 0.40f, 0.50f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.90f, 0.90f, 0.90f, 0.25f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(1.00f, 1.00f, 1.00f, 0.33f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
    colors[ImGuiCol_TabActive]              = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_TabUnfocused]           = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
    colors[ImGuiCol_DockingPreview]         = ImVec4(1.00f, 0.50f, 0.00f, 0.50f);
    colors[ImGuiCol_DockingEmptyBg]         = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_PlotLines]              = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.50f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.50f, 0.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.50f, 0.50f, 0.50f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 0.50f, 0.00f, 0.90f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(1.00f, 0.50f, 0.00f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 0.50f, 0.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.20f, 0.20f, 0.20f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
}