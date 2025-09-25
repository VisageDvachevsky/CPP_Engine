#include "ToolBar.h"
#include "editor/Editor.h"
#include "editor/TransformGizmo.h"
#include "scene/Scene.h"
#include "scene/Camera.h"
#include "renderer/Renderer.h"

#include <imgui.h>

ToolBar::ToolBar(Editor& editor) : m_editor(editor) {
}

void ToolBar::show(Scene& scene, Camera& camera, Renderer& renderer) {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | 
                            ImGuiWindowFlags_NoMove | 
                            ImGuiWindowFlags_NoResize |
                            ImGuiWindowFlags_NoSavedSettings;
    
    ImGui::Begin("ToolBar", nullptr, flags);
    
    showTransformTools();
    ImGui::SameLine();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();
    
    showRenderSettings(renderer);
    ImGui::SameLine();
    ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
    ImGui::SameLine();
    
    showCameraControls(camera);
    
    ImGui::End();
}

void ToolBar::showTransformTools() {
    TransformGizmo& gizmo = m_editor.getGizmo();
    
    // Transform mode buttons
    bool isTranslate = (gizmo.getMode() == GizmoMode::Translate);
    bool isRotate = (gizmo.getMode() == GizmoMode::Rotate);
    bool isScale = (gizmo.getMode() == GizmoMode::Scale);
    
    if (ImGui::Button("Move", ImVec2(50, 0)) || (ImGui::IsKeyPressed(ImGuiKey_W) && !ImGui::GetIO().WantTextInput)) {
        gizmo.setMode(GizmoMode::Translate);
    }
    if (isTranslate) {
        ImGui::GetWindowDrawList()->AddRect(
            ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
            IM_COL32(255, 165, 0, 255), 0.0f, 0, 2.0f);
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Rotate", ImVec2(50, 0)) || (ImGui::IsKeyPressed(ImGuiKey_E) && !ImGui::GetIO().WantTextInput)) {
        gizmo.setMode(GizmoMode::Rotate);
    }
    if (isRotate) {
        ImGui::GetWindowDrawList()->AddRect(
            ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
            IM_COL32(255, 165, 0, 255), 0.0f, 0, 2.0f);
    }
    
    ImGui::SameLine();
    if (ImGui::Button("Scale", ImVec2(50, 0)) || (ImGui::IsKeyPressed(ImGuiKey_R) && !ImGui::GetIO().WantTextInput)) {
        gizmo.setMode(GizmoMode::Scale);
    }
    if (isScale) {
        ImGui::GetWindowDrawList()->AddRect(
            ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
            IM_COL32(255, 165, 0, 255), 0.0f, 0, 2.0f);
    }
    
    // Space toggle
    ImGui::SameLine();
    bool isWorld = (gizmo.getSpace() == GizmoSpace::World);
    const char* spaceText = isWorld ? "World" : "Local";
    if (ImGui::Button(spaceText, ImVec2(50, 0))) {
        gizmo.setSpace(isWorld ? GizmoSpace::Local : GizmoSpace::World);
    }
    
    // Snap toggle
    ImGui::SameLine();
    bool snapEnabled = gizmo.isSnapEnabled();
    if (ImGui::Checkbox("Snap", &snapEnabled)) {
        gizmo.setSnap(snapEnabled);
    }
    
    if (snapEnabled) {
        ImGui::SameLine();
        float snapValue = gizmo.getSnapValue();
        ImGui::SetNextItemWidth(60);
        if (ImGui::DragFloat("##SnapValue", &snapValue, 0.1f, 0.1f, 10.0f, "%.1f")) {
            gizmo.setSnapValue(snapValue);
        }
    }
}

void ToolBar::showRenderSettings(Renderer& renderer) {
    // Samples per pixel
    int spp = renderer.getSamplesPerPixel();
    ImGui::SetNextItemWidth(80);
    if (ImGui::DragInt("SPP", &spp, 1, 1, 64)) {
        renderer.setSamplesPerPixel(spp);
    }
    
    ImGui::SameLine();
    
    // Max bounces
    int bounces = renderer.getMaxBounces();
    ImGui::SetNextItemWidth(80);
    if (ImGui::DragInt("Bounces", &bounces, 1, 1, 16)) {
        renderer.setMaxBounces(bounces);
    }
    
    // Quick presets
    ImGui::SameLine();
    if (ImGui::Button("Fast")) {
        renderer.setSamplesPerPixel(4);
        renderer.setMaxBounces(4);
    }
    ImGui::SameLine();
    if (ImGui::Button("Quality")) {
        renderer.setSamplesPerPixel(32);
        renderer.setMaxBounces(12);
    }
}

void ToolBar::showCameraControls(Camera& camera) {
    if (ImGui::Button("Reset Camera")) {
        camera.reset();
    }
    
    ImGui::SameLine();
    
    // FOV control
    float fov = camera.getFov();
    ImGui::SetNextItemWidth(80);
    if (ImGui::DragFloat("FOV", &fov, 1.0f, 10.0f, 120.0f, "%.0f°")) {
        camera.setFov(fov);
    }
}