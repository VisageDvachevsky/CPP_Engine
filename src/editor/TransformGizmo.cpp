#include "TransformGizmo.h"
#include "scene/Object.h"
#include "scene/Camera.h"
#include "renderer/Renderer.h"
#include "core/Input.h"
#include <ImGuizmo.h>
#include <imgui.h>
#include <GLFW/glfw3.h>

TransformGizmo::TransformGizmo() {
}

void TransformGizmo::update(Object& object, Camera& camera) {
    updateGizmoMatrices(object, camera);
    handleGizmoInteraction(object, camera);
}

void TransformGizmo::render(Renderer& renderer, Camera& camera) {
    // ImGuizmo will be rendered in the GUI system
    // This method is for custom gizmo rendering if needed
}

void TransformGizmo::handleGizmoInteraction(Object& object, Camera& camera) {
    // Get viewport info
    ImGuiIO& io = ImGui::GetIO();
    
    // Set up ImGuizmo
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist();
    
    // Get window position and size
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();
    
    ImGuizmo::SetRect(windowPos.x, windowPos.y, windowSize.x, windowSize.y);
    
    // Get matrices
    Mat4 view = camera.getViewMatrix();
    Mat4 proj = camera.getProjectionMatrix(windowSize.x / windowSize.y);
    
    // Object transform matrix
    Transform& transform = object.getTransform();
    Mat4 model = Mat4::translate(transform.position) * 
                 Mat4::scale(transform.scale);
    
    // Convert to ImGuizmo format (column-major)
    float modelMatrix[16];
    float viewMatrix[16];
    float projMatrix[16];
    
    for (int i = 0; i < 16; ++i) {
        modelMatrix[i] = model.data()[i];
        viewMatrix[i] = view.data()[i];
        projMatrix[i] = proj.data()[i];
    }
    
    // Determine operation
    ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
    switch (m_mode) {
        case GizmoMode::Translate: operation = ImGuizmo::TRANSLATE; break;
        case GizmoMode::Rotate: operation = ImGuizmo::ROTATE; break;
        case GizmoMode::Scale: operation = ImGuizmo::SCALE; break;
    }
    
    // Determine space
    ImGuizmo::MODE mode = (m_space == GizmoSpace::World) ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
    
    // Enable snapping if needed
    float snap[3] = {m_snapValue, m_snapValue, m_snapValue};
    float* snapPtr = m_snapEnabled ? snap : nullptr;
    
    // Handle the gizmo
    bool wasManipulated = ImGuizmo::Manipulate(
        viewMatrix, projMatrix,
        operation, mode,
        modelMatrix,
        nullptr, // delta matrix
        snapPtr
    );
    
    // Update object if gizmo was used
    if (wasManipulated) {
        // Extract transform from matrix
        Vec3 translation, rotation, scale;
        ImGuizmo::DecomposeMatrixToComponents(
            modelMatrix,
            translation.data(),
            rotation.data(),
            scale.data()
        );
        
        transform.position = translation;
        transform.rotation = rotation;
        transform.scale = scale;
    }
    
    // Update state
    m_isActive = ImGuizmo::IsUsing();
    m_isHovered = ImGuizmo::IsOver();
}

void TransformGizmo::updateGizmoMatrices(const Object& object, const Camera& camera) {
    const Transform& transform = object.getTransform();
    m_gizmoPosition = transform.position;
    
    // Calculate gizmo size based on distance to camera
    float distance = (camera.getPosition() - m_gizmoPosition).length();
    m_gizmoSize = distance * 0.1f; // Scale with distance
    
    // Create gizmo transformation matrix
    m_gizmoMatrix = Mat4::translate(m_gizmoPosition) * Mat4::scale(Vec3{m_gizmoSize});
}

Vec3 TransformGizmo::snapVector(const Vec3& value) const {
    if (!m_snapEnabled) return value;
    
    return Vec3{
        snapFloat(value.x),
        snapFloat(value.y),
        snapFloat(value.z)
    };
}

float TransformGizmo::snapFloat(float value) const {
    if (!m_snapEnabled) return value;
    
    return std::round(value / m_snapValue) * m_snapValue;
}