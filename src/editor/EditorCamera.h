#pragma once

#include "math/Vec3.h"

class Camera;

enum class CameraMode {
    Orbit,
    Fly,
    Focus
};

class EditorCamera {
public:
    EditorCamera(Camera& camera);
    ~EditorCamera() = default;
    
    void update(float dt, bool isViewportFocused);
    
    void setMode(CameraMode mode) { m_mode = mode; }
    CameraMode getMode() const { return m_mode; }
    
    void focusOnObject(const Vec3& position, float radius = 1.0f);
    void frameSelection(const Vec3& minBounds, const Vec3& maxBounds);
    
    void reset();
    
    // Settings
    void setMovementSpeed(float speed) { m_movementSpeed = speed; }
    void setRotationSpeed(float speed) { m_rotationSpeed = speed; }
    void setZoomSpeed(float speed) { m_zoomSpeed = speed; }

private:
    void updateOrbitCamera(float dt);
    void updateFlyCamera(float dt);
    void handleMouseInput(float dt);
    void handleKeyboardInput(float dt);
    
    Camera& m_camera;
    CameraMode m_mode = CameraMode::Orbit;
    
    // Orbit camera state
    Vec3 m_orbitTarget{0, 0, 0};
    float m_orbitDistance = 10.0f;
    float m_orbitYaw = -90.0f;
    float m_orbitPitch = 20.0f;
    
    // Camera settings
    float m_movementSpeed = 5.0f;
    float m_rotationSpeed = 0.5f;
    float m_zoomSpeed = 2.0f;
    float m_mouseSensitivity = 0.1f;
    
    // Input state
    bool m_isRotating = false;
    bool m_isPanning = false;
    bool m_isZooming = false;
    
    Vec3 m_lastMousePos{0, 0, 0};
};