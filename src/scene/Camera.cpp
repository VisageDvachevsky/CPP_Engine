#include "Camera.h"
#include "core/Input.h"
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

Camera::Camera() {
    updateVectors();
}

void Camera::update(float dt) {
    // Basic camera update - detailed control is handled by EditorCamera
}

void Camera::reset() {
    m_target = Vec3{0, 0, 0};
    m_distance = 5.0f;
    m_yaw = -90.0f;
    m_pitch = 20.0f;
    m_fov = 45.0f;
    updateVectors();
}

void Camera::rotate(float yaw, float pitch) {
    m_yaw += yaw;
    m_pitch += pitch;
    
    // Constrain pitch
    m_pitch = std::clamp(m_pitch, -89.0f, 89.0f);
    
    updateVectors();
}

void Camera::zoom(float delta) {
    m_distance += delta;
    m_distance = std::max(0.1f, m_distance);
    updateVectors();
}

void Camera::pan(float deltaX, float deltaY) {
    Vec3 right = m_right;
    Vec3 up = m_up;
    
    m_target += right * deltaX + up * deltaY;
    updateVectors();
}

Vec3 Camera::getPosition() const {
    return m_position;
}

void Camera::setPosition(const Vec3& pos) {
    m_position = pos;
    updateVectors();
}

void Camera::lookAt(const Vec3& target) {
    m_target = target;
    
    Vec3 direction = (m_position - target).normalized();
    m_pitch = std::asin(direction.y) * 180.0f / M_PI;
    m_yaw = std::atan2(direction.z, direction.x) * 180.0f / M_PI;
    
    updateVectors();
}

void Camera::updateVectors() {
    // Convert spherical to cartesian coordinates
    float yawRad = m_yaw * M_PI / 180.0f;
    float pitchRad = m_pitch * M_PI / 180.0f;
    
    Vec3 direction;
    direction.x = std::cos(yawRad) * std::cos(pitchRad);
    direction.y = std::sin(pitchRad);
    direction.z = std::sin(yawRad) * std::cos(pitchRad);
    
    m_front = direction.normalized();
    m_position = m_target + m_front * m_distance;
    
    // Calculate right and up vectors
    m_right = cross(m_front, m_worldUp).normalized();
    m_up = cross(m_right, m_front).normalized();
}

Mat4 Camera::getViewMatrix() const {
    return Mat4::lookAt(m_position, m_target, m_up);
}

Mat4 Camera::getProjectionMatrix(float aspect) const {
    return Mat4::perspective(m_fov * M_PI / 180.0f, aspect, 0.1f, 100.0f);
}

Ray Camera::screenPointToRay(const Vec2& screenPoint) const {
    return screenPointToRay(screenPoint, m_lastScreenSize);
}

Ray Camera::screenPointToRay(const Vec2& screenPoint, const Vec2& screenSize) const {
    m_lastScreenSize = screenSize;
    
    // Convert screen coordinates to NDC (-1 to 1)
    float x = (2.0f * screenPoint.x) / screenSize.x - 1.0f;
    float y = 1.0f - (2.0f * screenPoint.y) / screenSize.y;
    
    // Create ray direction in camera space
    float aspect = screenSize.x / screenSize.y;
    float tanHalfFov = std::tan(m_fov * 0.5f * M_PI / 180.0f);
    
    Vec3 rayDir;
    rayDir.x = x * aspect * tanHalfFov;
    rayDir.y = y * tanHalfFov;
    rayDir.z = -1.0f; // Forward direction
    
    // Transform to world space
    Vec3 worldRayDir = rayDir.x * m_right + rayDir.y * m_up + rayDir.z * (-m_front);
    worldRayDir.normalize();
    
    return Ray(m_position, worldRayDir);
}