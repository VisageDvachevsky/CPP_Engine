#include "TransformGizmo.h"
#include "scene/Object.h"
#include "scene/Camera.h"
#include "renderer/Renderer.h"
#include "core/Input.h"
#include "core/Logger.h"
#include <imgui.h>     
#include <ImGuizmo.h>   
#include <GLFW/glfw3.h>

TransformGizmo::TransformGizmo() {
    LOG_INFO("TransformGizmo initialized");
}

void TransformGizmo::update(Object& object, Camera& camera) {
    updateGizmoMatrices(object, camera);
    handleGizmoInteraction(object, camera);
}

void TransformGizmo::render(Renderer& renderer, Camera& camera) {
    // ImGuizmo rendering is handled in the ImGui update step
}

void TransformGizmo::handleGizmoInteraction(Object& object, Camera& camera) {
    ImGuiIO& io = ImGui::GetIO();
    
    // Don't handle interaction if ImGui wants to capture mouse
    if (io.WantCaptureMouse) {
        return;
    }
    
    // Make sure ImGuizmo is initialized at the start of each frame
    ImGuizmo::BeginFrame();
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist();
    
    // Get the current viewport window
    ImVec2 windowPos = ImGui::GetWindowPos();
    ImVec2 windowSize = ImGui::GetWindowSize();
    
    // Make sure we're working with valid window dimensions
    if (windowSize.x <= 0 || windowSize.y <= 0) {
        LOG_DEBUG("Invalid window size for gizmo: [{:.1f}, {:.1f}]", windowSize.x, windowSize.y);
        return;
    }
    
    // Set the gizmo area
    ImGuizmo::SetRect(windowPos.x, windowPos.y, windowSize.x, windowSize.y);
    
    // Get camera matrices
    Mat4 view = camera.getViewMatrix();
    Mat4 proj = camera.getProjectionMatrix(windowSize.x / windowSize.y);
    
    // Get object transform matrix
    Transform& transform = object.getTransform();
    Mat4 model = transform.getMatrix();
    
    // Convert to ImGuizmo format (column-major)
    float modelMatrix[16];
    float viewMatrix[16];
    float projMatrix[16];
    
    // Copy matrices to float arrays for ImGuizmo
    for (int i = 0; i < 16; ++i) {
        modelMatrix[i] = model.data()[i];
        viewMatrix[i] = view.data()[i];
        projMatrix[i] = proj.data()[i];
    }
    
    // Determine operation and mode based on current gizmo settings
    ImGuizmo::OPERATION operation;
    switch (m_mode) {
        case GizmoMode::Translate: 
            operation = ImGuizmo::TRANSLATE; 
            break;
        case GizmoMode::Rotate: 
            operation = ImGuizmo::ROTATE; 
            break;
        case GizmoMode::Scale: 
            operation = ImGuizmo::SCALE; 
            break;
        default:
            operation = ImGuizmo::TRANSLATE;
    }
    
    ImGuizmo::MODE mode = (m_space == GizmoSpace::World) ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
    
    // Set up snapping
    float snap[3] = {m_snapValue, m_snapValue, m_snapValue};
    float* snapPtr = m_snapEnabled ? snap : nullptr;
    
    // Draw the gizmo and check if it was manipulated
    bool wasManipulated = ImGuizmo::Manipulate(
        viewMatrix, projMatrix,
        operation, mode,
        modelMatrix,
        nullptr, // delta matrix
        snapPtr
    );
    
    // If gizmo was manipulated, update the object transform
    if (wasManipulated) {
        LOG_DEBUG("Gizmo manipulated, updating object transform");
        
        // Extract transform from matrix
        Vec3 translation, rotation, scale;
        ImGuizmo::DecomposeMatrixToComponents(
            modelMatrix,
            translation.data(),
            rotation.data(),
            scale.data()
        );
        
        // Update object transform
        transform.position = translation;
        transform.rotation = rotation;
        transform.scale = scale;
        
        // Log transformation for debugging
        LOG_DEBUG("Updated transform: pos=[{:.2f}, {:.2f}, {:.2f}], rot=[{:.2f}, {:.2f}, {:.2f}], scale=[{:.2f}, {:.2f}, {:.2f}]", 
                 translation.x, translation.y, translation.z,
                 rotation.x, rotation.y, rotation.z,
                 scale.x, scale.y, scale.z);
    }
    
    // Update gizmo state
    m_isActive = ImGuizmo::IsUsing();
    m_isHovered = ImGuizmo::IsOver();
    
    // Add debug info if state changes
    if (m_isActive != m_wasActive) {
        if (m_isActive) {
            LOG_DEBUG("Gizmo activated");
        } else {
            LOG_DEBUG("Gizmo deactivated");
        }
        m_wasActive = m_isActive;
    }
    
    if (m_isHovered != m_wasHovered) {
        if (m_isHovered) {
            LOG_DEBUG("Gizmo hovered");
        } else {
            LOG_DEBUG("Gizmo unhovered");
        }
        m_wasHovered = m_isHovered;
    }
}

void TransformGizmo::updateGizmoMatrices(const Object& object, const Camera& camera) {
    const Transform& transform = object.getTransform();
    m_gizmoPosition = transform.position;
    
    // Scale gizmo based on distance from camera for better visibility
    float distance = (camera.getPosition() - m_gizmoPosition).length();
    m_gizmoSize = distance * 0.15f; // Increase size factor for better visibility
    
    // Apply minimum size to ensure gizmo is always visible
    m_gizmoSize = std::max(m_gizmoSize, 0.5f);
    
    m_gizmoMatrix = Mat4::translate(m_gizmoPosition) * Mat4::scale(Vec3{m_gizmoSize});
    
    // Log gizmo update
    LOG_DEBUG("Gizmo updated: position=[{:.2f}, {:.2f}, {:.2f}], size={:.2f}", 
             m_gizmoPosition.x, m_gizmoPosition.y, m_gizmoPosition.z, m_gizmoSize);
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