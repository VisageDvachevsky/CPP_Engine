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
    
    // Если ImGui захватил ввод, игнорируем его для viewport
    if (io.WantCaptureMouse) {
        return;
    }
    
    ImVec2 mousePos = ImGui::GetMousePos();
    Vec2 viewportMousePos = Vec2{mousePos.x - m_viewportPos.x, mousePos.y - m_viewportPos.y};
    
    // Проверяем, находится ли мышь внутри вьюпорта
    bool mouseInViewport = viewportMousePos.x >= 0 && viewportMousePos.y >= 0 && 
                           viewportMousePos.x < m_viewportSize.x && viewportMousePos.y < m_viewportSize.y;
                           
    if (!mouseInViewport) {
        return;
    }
    
    // Обрабатываем клики мыши
    if (Input::isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) || 
        Input::isMouseButtonDoubleClicked(GLFW_MOUSE_BUTTON_LEFT)) {
        
        // Показываем в логе тип события
        if (Input::isMouseButtonDoubleClicked(GLFW_MOUSE_BUTTON_LEFT)) {
            LOG_INFO("Viewport: Double click detected at ({}, {})", viewportMousePos.x, viewportMousePos.y);
        } else {
            LOG_INFO("Viewport: Single click detected at ({}, {})", viewportMousePos.x, viewportMousePos.y);
        }
        
        // В любом случае отправляем событие в SelectionManager
        if (!ImGuizmo::IsOver()) {
            m_editor.getSelection().handleMousePicking(viewportMousePos, camera);
        }
    }
}

void Viewport::renderViewportContent(Scene& scene, Camera& camera, Renderer& renderer) {
    m_framebuffer->bind();
    
    glViewport(0, 0, static_cast<int>(m_viewportSize.x), static_cast<int>(m_viewportSize.y));

    renderer.setViewportSize(static_cast<int>(m_viewportSize.x), static_cast<int>(m_viewportSize.y));
    
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
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
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(m_viewportPos.x, m_viewportPos.y, m_viewportSize.x, m_viewportSize.y);
    
    if (m_editor.getSelection().hasSelection()) {
        Object* selectedObject = m_editor.getSelection().getSelectedObject();
        if (selectedObject) {
            m_editor.getGizmo().update(*selectedObject, camera);
        }
    }
}