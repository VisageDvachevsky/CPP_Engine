#pragma once

#include "math/Vec3.h"
#include "math/Mat4.h"

class Object;
class Camera;
class Renderer;

enum class GizmoMode {
    Translate,
    Rotate,
    Scale
};

enum class GizmoSpace {
    World,
    Local
};

class TransformGizmo {
public:
    TransformGizmo();
    ~TransformGizmo() = default;
    
    void update(Object& object, Camera& camera);
    void render(Renderer& renderer, Camera& camera);
    
    // Mode and space
    void setMode(GizmoMode mode) { m_mode = mode; }
    GizmoMode getMode() const { return m_mode; }
    
    void setSpace(GizmoSpace space) { m_space = space; }
    GizmoSpace getSpace() const { return m_space; }
    
    // Settings
    void setSnap(bool enabled) { m_snapEnabled = enabled; }
    bool isSnapEnabled() const { return m_snapEnabled; }
    
    void setSnapValue(float value) { m_snapValue = value; }
    float getSnapValue() const { return m_snapValue; }
    
    // Interaction
    bool isActive() const { return m_isActive; }
    bool isHovered() const { return m_isHovered; }

private:
    void handleGizmoInteraction(Object& object, Camera& camera);
    void updateGizmoMatrices(const Object& object, const Camera& camera);
    
    Vec3 snapVector(const Vec3& value) const;
    float snapFloat(float value) const;
    
    GizmoMode m_mode = GizmoMode::Translate;
    GizmoSpace m_space = GizmoSpace::World;
    
    bool m_snapEnabled = false;
    float m_snapValue = 1.0f;
    
    bool m_isActive = false;
    bool m_isHovered = false;
    
    // Added tracking for state changes
    bool m_wasActive = false;
    bool m_wasHovered = false;
    
    // Gizmo state
    Mat4 m_gizmoMatrix;
    Vec3 m_gizmoPosition{0, 0, 0};
    float m_gizmoSize = 1.0f;
    
    // Interaction state
    Vec3 m_lastMouseWorldPos{0, 0, 0};
    Vec3 m_startObjectPosition{0, 0, 0};
    Vec3 m_startObjectRotation{0, 0, 0};
    Vec3 m_startObjectScale{1, 1, 1};
};