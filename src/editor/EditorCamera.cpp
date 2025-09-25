#include "EditorCamera.h"
#include "core/Input.h"
#include "scene/Camera.h"
#include "math/Math.h"
#include <GLFW/glfw3.h>
#include <algorithm>

EditorCamera::EditorCamera(Camera& camera) : m_camera(camera) {
    reset();
}

void EditorCamera::update(float dt, bool isViewportFocused) {
    if (!isViewportFocused) return;
    
    switch (m_mode) {
        case CameraMode::Orbit:
            updateOrbitCamera(dt);
            break;
        case CameraMode::Fly:
            updateFlyCamera(dt);
            break;
        case CameraMode::Focus:
            break;
    }
}

void EditorCamera::updateOrbitCamera(float dt) {
    Vec2 mouseDelta = Input::getMouseDelta();
    float scrollDelta = Input::getScrollDelta();
    
    // Right mouse button for rotation
    if (Input::isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
        if (!m_isRotating) {
            m_isRotating = true;
            Input::setMouseCursorEnabled(false);
        }
        
        m_orbitYaw += mouseDelta.x * m_mouseSensitivity;
        m_orbitPitch -= mouseDelta.y * m_mouseSensitivity;
        
        m_orbitPitch = std::clamp(m_orbitPitch, -89.0f, 89.0f);
    } else if (m_isRotating) {
        m_isRotating = false;
        Input::setMouseCursorEnabled(true);
    }
    
    // Middle mouse button for panning
    if (Input::isMouseButtonPressed(GLFW_MOUSE_BUTTON_MIDDLE)) {
        if (!m_isPanning) {
            m_isPanning = true;
        }
        
        Vec3 right = m_camera.getRight();
        Vec3 up = m_camera.getUp();
        
        float panSpeed = m_orbitDistance * 0.001f;
        Vec3 panDelta = right * (-mouseDelta.x * panSpeed) + up * (mouseDelta.y * panSpeed);
        m_orbitTarget += panDelta;
    } else {
        m_isPanning = false;
    }
    
    // Scroll wheel for zoom
    if (scrollDelta != 0.0f) {
        m_orbitDistance -= scrollDelta * m_zoomSpeed * (m_orbitDistance * 0.1f);
        m_orbitDistance = std::max(0.1f, m_orbitDistance);
    }
    
    updateOrbitCameraPosition();
}

void EditorCamera::updateFlyCamera(float dt) {
    Vec2 mouseDelta = Input::getMouseDelta();
    
    // Right mouse button enables fly mode
    if (Input::isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
        if (!m_isRotating) {
            m_isRotating = true;
            Input::setMouseCursorEnabled(false);
        }
        
        // Mouse look
        m_orbitYaw += mouseDelta.x * m_mouseSensitivity;
        m_orbitPitch -= mouseDelta.y * m_mouseSensitivity;
        m_orbitPitch = std::clamp(m_orbitPitch, -89.0f, 89.0f);
        
        // WASD movement
        Vec3 movement{0, 0, 0};
        Vec3 forward = m_camera.getDirection();
        Vec3 right = m_camera.getRight();
        Vec3 up = Vec3{0, 1, 0};
        
        if (Input::isKeyHeld(GLFW_KEY_W)) movement += forward;
        if (Input::isKeyHeld(GLFW_KEY_S)) movement -= forward;
        if (Input::isKeyHeld(GLFW_KEY_D)) movement += right;
        if (Input::isKeyHeld(GLFW_KEY_A)) movement -= right;
        if (Input::isKeyHeld(GLFW_KEY_E)) movement += up;
        if (Input::isKeyHeld(GLFW_KEY_Q)) movement -= up;
        
        float speed = m_movementSpeed;
        if (Input::isKeyHeld(GLFW_KEY_LEFT_SHIFT)) speed *= 3.0f;
        if (Input::isKeyHeld(GLFW_KEY_LEFT_CONTROL)) speed *= 0.3f;
        
        if (movement.lengthSq() > 0) {
            movement.normalize();
            Vec3 currentPos = m_camera.getPosition();
            m_camera.setPosition(currentPos + movement * speed * dt);
        }
        
        updateFlyCameraOrientation();
    } else if (m_isRotating) {
        m_isRotating = false;
        Input::setMouseCursorEnabled(true);
    }
}

void EditorCamera::updateOrbitCameraPosition() {
    float yawRad = m_orbitYaw * (3.14159f / 180.0f);
    float pitchRad = m_orbitPitch * (3.14159f / 180.0f);
    
    Vec3 offset;
    offset.x = cos(yawRad) * cos(pitchRad);
    offset.y = sin(pitchRad);
    offset.z = sin(yawRad) * cos(pitchRad);
    
    Vec3 position = m_orbitTarget + offset * m_orbitDistance;
    m_camera.setPosition(position);
    m_camera.lookAt(m_orbitTarget);
}

void EditorCamera::updateFlyCameraOrientation() {
    float yawRad = m_orbitYaw * (3.14159f / 180.0f);
    float pitchRad = m_orbitPitch * (3.14159f / 180.0f);
    
    Vec3 direction;
    direction.x = cos(yawRad) * cos(pitchRad);
    direction.y = sin(pitchRad);
    direction.z = sin(yawRad) * cos(pitchRad);
    
    Vec3 position = m_camera.getPosition();
    m_camera.lookAt(position + direction);
}

void EditorCamera::focusOnObject(const Vec3& position, float radius) {
    m_orbitTarget = position;
    m_orbitDistance = std::max(radius * 3.0f, 2.0f);
    updateOrbitCameraPosition();
}

void EditorCamera::frameSelection(const Vec3& minBounds, const Vec3& maxBounds) {
    Vec3 center = (minBounds + maxBounds) * 0.5f;
    Vec3 size = maxBounds - minBounds;
    float radius = size.length() * 0.5f;
    
    focusOnObject(center, radius);
}

void EditorCamera::reset() {
    m_orbitTarget = Vec3{0, 0, 0};
    m_orbitDistance = 10.0f;
    m_orbitYaw = -90.0f;
    m_orbitPitch = 20.0f;
    m_mode = CameraMode::Orbit;
    
    updateOrbitCameraPosition();
}