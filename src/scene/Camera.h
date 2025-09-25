#pragma once

#include "math/Vec3.h"
#include "math/Vec2.h"
#include "math/Mat4.h"
#include "math/Ray.h"

class Camera {
public:
    Camera();
    ~Camera() = default;
    
    void update(float dt);
    void reset();
    
    // Orbital controls
    void rotate(float yaw, float pitch);
    void zoom(float delta);
    void pan(float deltaX, float deltaY);
    
    // Getters
    Vec3 getPosition() const;
    Vec3 getDirection() const { return m_front; }
    Vec3 getUp() const { return m_up; }
    Vec3 getRight() const { return m_right; }
    
    float getFov() const { return m_fov; }
    void setFov(float fov) { m_fov = fov; updateVectors(); }
    
    Mat4 getViewMatrix() const;
    Mat4 getProjectionMatrix(float aspect) const;
    
    void setPosition(const Vec3& pos);
    void lookAt(const Vec3& target);
    
    // Orbital camera settings
    void setTarget(const Vec3& target) { m_target = target; updateVectors(); }
    Vec3 getTarget() const { return m_target; }
    
    float getDistance() const { return m_distance; }
    void setDistance(float distance) { m_distance = distance; updateVectors(); }
    
    // Ray casting
    Ray screenPointToRay(const Vec2& screenPoint) const;
    Ray screenPointToRay(const Vec2& screenPoint, const Vec2& screenSize) const;

private:
    void updateVectors();
    
    // Orbital camera parameters
    Vec3 m_target{0, 0, 0};
    float m_distance = 5.0f;
    float m_yaw = -90.0f;
    float m_pitch = 20.0f;
    
    // Camera vectors
    Vec3 m_position{0, 2, 5};
    Vec3 m_front{0, 0, -1};
    Vec3 m_up{0, 1, 0};
    Vec3 m_right{1, 0, 0};
    Vec3 m_worldUp{0, 1, 0};
    
    // Camera settings
    float m_fov = 45.0f;
    float m_movementSpeed = 5.0f;
    float m_mouseSensitivity = 0.1f;
    
    // For ray casting
    mutable Vec2 m_lastScreenSize{1920, 1080};
    
    // Input state
    bool m_firstMouse = true;
};