#include "Viewport.h"
#include "editor/Editor.h"
#include "editor/SelectionManager.h"
#include "editor/TransformGizmo.h"
#include "scene/Scene.h"
#include "scene/Camera.h"
#include "renderer/Renderer.h"
#include "renderer/Framebuffer.h"
#include "core/Input.h"
#include "core/Logger.h"

#include <imgui.h>
#include <ImGuizmo.h>
#include <GLFW/glfw3.h>

Viewport::Viewport(Editor& editor) : m_editor(editor) {
    m_framebuffer = std::make_unique<Framebuffer>(
        static_cast<int>(m_viewportSize.x), 
        static_cast<int>(m_viewportSize.y)
    );
}

Viewport::~Viewport() = default;

void Viewport::show(Scene& scene, Camera& camera, Renderer& renderer) {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    
    if (ImGui::Begin("Viewport")) {
        m_isFocused = ImGui::IsWindowFocused();
        m_isHovered = ImGui::IsWindowHovered();
        
        ImVec2 viewportSize = ImGui::GetContentRegionAvail();
        if (viewportSize.x > 0 && viewportSize.y > 0) {
            if (viewportSize.x != m_viewportSize.x || viewportSize.y != m_viewportSize.y) {
                m_viewportSize = Vec2{viewportSize.x, viewportSize.y};
                m_framebuffer->resize(static_cast<int>(viewportSize.x), static_cast<int>(viewportSize.y));
                LOG_DEBUG("Viewport resized to {}x{}", viewportSize.x, viewportSize.y);
            }
            
            // Get viewport position
            ImVec2 viewportPos = ImGui::GetCursorScreenPos();
            m_viewportPos = Vec2{viewportPos.x, viewportPos.y};
            
            if (m_isFocused || m_isHovered) {
                handleInput(scene, camera);
            }
            
            renderViewportContent(scene, camera, renderer);
            
            ImGui::Image(
                reinterpret_cast<void*>(m_framebuffer->getColorTexture()), 
                ImVec2(m_viewportSize.x, m_viewportSize.y),
                ImVec2(0, 1), ImVec2(1, 0) // Flip Y coordinate
            );
            
            if (m_isFocused) {
                renderGizmos(scene, camera);
            }
            
            renderOverlays(scene, camera, renderer);
        }
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

void Viewport::onResize(int width, int height) {
    // Handled in show() method
}

void Viewport::handleInput(Scene& scene, Camera& camera) {
    ImGuiIO& io = ImGui::GetIO();
    
    // If ImGui captured input, ignore it for viewport
    if (io.WantCaptureMouse) {
        return;
    }
    
    // Get mouse position relative to viewport
    ImVec2 mousePos = ImGui::GetMousePos();
    Vec2 viewportMousePos = Vec2{mousePos.x - m_viewportPos.x, mousePos.y - m_viewportPos.y};
    
    // Check if mouse is inside viewport
    bool mouseInViewport = viewportMousePos.x >= 0 && viewportMousePos.y >= 0 && 
                         viewportMousePos.x < m_viewportSize.x && viewportMousePos.y < m_viewportSize.y;
                         
    if (!mouseInViewport) {
        return;
    }
    
    // Process mouse buttons
    bool isMouseButtonLeftPressed = Input::isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
    bool isMouseButtonRightPressed = Input::isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT);
    bool isDoubleClick = Input::isMouseButtonDoubleClicked(GLFW_MOUSE_BUTTON_LEFT);
    
    // Log mouse interactions for debugging
    if (isMouseButtonLeftPressed) {
        LOG_DEBUG("Left click in viewport at ({:.1f}, {:.1f})", viewportMousePos.x, viewportMousePos.y);
    }
    
    if (isDoubleClick) {
        LOG_DEBUG("Double click in viewport at ({:.1f}, {:.1f})", viewportMousePos.x, viewportMousePos.y);
    }
    
    // Handle object selection only if left mouse button pressed or double clicked
    if ((isMouseButtonLeftPressed || isDoubleClick) && !ImGuizmo::IsOver()) {
        // Convert viewport coordinates to ray for selection
        Ray ray = camera.screenPointToRay(viewportMousePos, m_viewportSize);
        
        // Forward to selection manager with explicit double click state
        m_editor.getSelection().handleMousePicking(viewportMousePos, camera);
        
        // If double-click, automatically focus on selected object
        if (isDoubleClick) {
            m_editor.focusOnSelectedObject();
            
            // Also activate transform gizmo
            m_editor.activateGizmo();
        }
    }
}

void Viewport::renderViewportContent(Scene& scene, Camera& camera, Renderer& renderer) {
    m_framebuffer->bind();
    
    glViewport(0, 0, static_cast<int>(m_viewportSize.x), static_cast<int>(m_viewportSize.y));
    
    renderer.setViewportSize(static_cast<int>(m_viewportSize.x), static_cast<int>(m_viewportSize.y));
    
    // Clear with a slightly darker background for better visibility
    glClearColor(0.08f, 0.08f, 0.08f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Render the scene
    renderer.render(scene, camera);
    
    m_framebuffer->unbind();
}

void Viewport::renderOverlays(Scene& scene, Camera& camera, Renderer& renderer) {
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 canvasPos = ImGui::GetCursorScreenPos();
    ImVec2 canvasSize = ImGui::GetContentRegionAvail();
    
    ImVec2 origin = ImVec2(canvasPos.x + 50, canvasPos.y + canvasSize.y - 50);
    float axisLength = 30.0f;
    
    // X axis (red)
    drawList->AddLine(origin, ImVec2(origin.x + axisLength, origin.y), IM_COL32(255, 0, 0, 255), 2.0f);
    drawList->AddText(ImVec2(origin.x + axisLength + 5, origin.y - 8), IM_COL32(255, 0, 0, 255), "X");
    
    // Y axis (green)  
    drawList->AddLine(origin, ImVec2(origin.x, origin.y - axisLength), IM_COL32(0, 255, 0, 255), 2.0f);
    drawList->AddText(ImVec2(origin.x - 8, origin.y - axisLength - 15), IM_COL32(0, 255, 0, 255), "Y");
    
    // Z axis (blue) - drawn diagonally to simulate 3D
    ImVec2 zEnd = ImVec2(origin.x - axisLength * 0.7f, origin.y - axisLength * 0.7f);
    drawList->AddLine(origin, zEnd, IM_COL32(0, 0, 255, 255), 2.0f);
    drawList->AddText(ImVec2(zEnd.x - 15, zEnd.y - 8), IM_COL32(0, 0, 255, 255), "Z");
}

void Viewport::renderGizmos(Scene& scene, Camera& camera) {
    // Only render gizmos if the viewport is focused
    if (!m_isFocused) {
        return;
    }
    
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(m_viewportPos.x, m_viewportPos.y, m_viewportSize.x, m_viewportSize.y);
    
    // Check if there is a selection and gizmo is active
    if (m_editor.getSelection().hasSelection() && m_editor.isGizmoActive()) {
        Object* selectedObject = m_editor.getSelection().getSelectedObject();
        if (selectedObject) {
            LOG_DEBUG("Updating gizmo for object '{}'", selectedObject->getName());
            m_editor.getGizmo().update(*selectedObject, camera);
        }
    }
}