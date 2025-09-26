#pragma once

#include "math/Vec2.h"
#include "math/Vec3.h"
#include "math/Ray.h"
#include <vector>
#include <memory>
#include <functional>

class Scene;
class Object;
class Camera;
class Renderer;

// Callback types
using ObjectFocusCallback = std::function<void(const Vec3& position, float radius)>;
using GizmoActivateCallback = std::function<void()>;

struct SelectionInfo {
    Object* object = nullptr;
    float distance = 0.0f;
    Vec3 hitPoint{0, 0, 0};
};

class SelectionManager {
public:
    SelectionManager(Scene& scene);
    ~SelectionManager() = default;
    
    void update();
    void renderSelection(Renderer& renderer, Camera& camera);
    
    // Callback setters
    void setObjectFocusCallback(const ObjectFocusCallback& callback) { m_objectFocusCallback = callback; }
    void setGizmoActivateCallback(const GizmoActivateCallback& callback) { m_activateGizmoCallback = callback; }
    
    // Selection methods
    void selectObject(Object* object);
    void deselectAll();
    void addToSelection(Object* object);
    void removeFromSelection(Object* object);
    
    // Queries
    bool hasSelection() const { return !m_selectedObjects.empty(); }
    Object* getSelectedObject() const;
    const std::vector<Object*>& getSelectedObjects() const { return m_selectedObjects; }
    
    // Mouse picking
    Object* pickObject(const Ray& ray);
    void handleMousePicking(const Vec2& mousePos, const Camera& camera);
    
    // Bounds calculation
    void getSelectionBounds(Vec3& minBounds, Vec3& maxBounds) const;

private:
    // Ray intersection helpers
    bool rayIntersectSphere(const Ray& ray, const Vec3& center, float radius, float& distance);
    bool rayIntersectPlane(const Ray& ray, const Vec3& point, const Vec3& normal, float& distance);
    bool rayIntersectAABB(const Ray& ray, const Vec3& minBounds, const Vec3& maxBounds, float& distance);
    
    Scene& m_scene;
    std::vector<Object*> m_selectedObjects;
    Object* m_hoveredObject = nullptr;
    
    // Callbacks
    ObjectFocusCallback m_objectFocusCallback;
    GizmoActivateCallback m_activateGizmoCallback;
};