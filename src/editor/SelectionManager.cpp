#include "SelectionManager.h"
#include "core/Input.h"
#include "scene/Scene.h"
#include "scene/Object.h"
#include "scene/Camera.h"
#include "renderer/Renderer.h"
#include "math/Ray.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <imgui.h>

SelectionManager::SelectionManager(Scene& scene) : m_scene(scene) {
}

void SelectionManager::update() {
    if (!ImGui::GetIO().WantCaptureMouse) {
        if (Input::isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
            // TODO: Implement mouse picking when viewport system is ready
        }
    }
}

void SelectionManager::renderSelection(Renderer& renderer, Camera& camera) {
    for (Object* object : m_selectedObjects) {
        if (object) {
            renderer.renderSelectionOutline(*object, camera);
        }
    }
    
    if (m_hoveredObject && std::find(m_selectedObjects.begin(), m_selectedObjects.end(), m_hoveredObject) == m_selectedObjects.end()) {
        renderer.renderHoverOutline(*m_hoveredObject, camera);
    }
}

void SelectionManager::selectObject(Object* object) {
    if (!object) return;
    
    for (Object* obj : m_selectedObjects) {
        if (obj) obj->setSelected(false);
    }
    
    m_selectedObjects.clear();
    m_selectedObjects.push_back(object);
    object->setSelected(true);
    
    m_scene.setSelectedObject(object);
}

void SelectionManager::deselectAll() {
    for (Object* obj : m_selectedObjects) {
        if (obj) obj->setSelected(false);
    }
    
    m_selectedObjects.clear();
    m_scene.setSelectedObject(nullptr);
}

void SelectionManager::addToSelection(Object* object) {
    if (!object) return;
    
    auto it = std::find(m_selectedObjects.begin(), m_selectedObjects.end(), object);
    if (it == m_selectedObjects.end()) {
        m_selectedObjects.push_back(object);
        object->setSelected(true);
    }
}

void SelectionManager::removeFromSelection(Object* object) {
    if (!object) return;
    
    auto it = std::find(m_selectedObjects.begin(), m_selectedObjects.end(), object);
    if (it != m_selectedObjects.end()) {
        m_selectedObjects.erase(it);
        object->setSelected(false);
    }
}

Object* SelectionManager::getSelectedObject() const {
    return m_selectedObjects.empty() ? nullptr : m_selectedObjects[0];
}

Object* SelectionManager::pickObject(const Ray& ray) {
    float closestDistance = std::numeric_limits<float>::max();
    Object* closestObject = nullptr;
    
    const auto& objects = m_scene.getObjects();
    for (const auto& obj : objects) {
        float distance;
        bool hit = false;
        
        Vec3 position = obj->getTransform().position;
        Vec3 scale = obj->getTransform().scale;
        
        switch (obj->getType()) {
            case ObjectType::Sphere: {
                float radius = scale.x; // Assuming uniform scale
                hit = rayIntersectSphere(ray, position, radius, distance);
                break;
            }
            case ObjectType::Plane: {
                Vec3 normal{0, 1, 0}; // Default plane normal
                hit = rayIntersectPlane(ray, position, normal, distance);
                break;
            }
            case ObjectType::Cube: {
                Vec3 halfExtents = scale * 0.5f;
                Vec3 minBounds = position - halfExtents;
                Vec3 maxBounds = position + halfExtents;
                hit = rayIntersectAABB(ray, minBounds, maxBounds, distance);
                break;
            }
        }
        
        if (hit && distance < closestDistance && distance > 0.001f) {
            closestDistance = distance;
            closestObject = obj.get();
        }
    }
    
    return closestObject;
}

void SelectionManager::handleMousePicking(const Vec2& mousePos, const Camera& camera) {
    Ray ray = camera.screenPointToRay(mousePos);
    Object* hitObject = pickObject(ray);
    
    if (Input::isKeyHeld(GLFW_KEY_LEFT_CONTROL)) {
        // Multi-selection
        if (hitObject) {
            auto it = std::find(m_selectedObjects.begin(), m_selectedObjects.end(), hitObject);
            if (it != m_selectedObjects.end()) {
                removeFromSelection(hitObject);
            } else {
                addToSelection(hitObject);
            }
        }
    } else {
        // Single selection
        if (hitObject) {
            selectObject(hitObject);
        } else {
            deselectAll();
        }
    }
}

void SelectionManager::getSelectionBounds(Vec3& minBounds, Vec3& maxBounds) const {
    if (m_selectedObjects.empty()) {
        minBounds = maxBounds = Vec3{0, 0, 0};
        return;
    }
    
    minBounds = Vec3{std::numeric_limits<float>::max()};
    maxBounds = Vec3{std::numeric_limits<float>::lowest()};
    
    for (Object* obj : m_selectedObjects) {
        if (!obj) continue;
        
        Vec3 pos = obj->getTransform().position;
        Vec3 scale = obj->getTransform().scale;
        
        Vec3 objMin = pos - scale * 0.5f;
        Vec3 objMax = pos + scale * 0.5f;
        
        minBounds.x = std::min(minBounds.x, objMin.x);
        minBounds.y = std::min(minBounds.y, objMin.y);
        minBounds.z = std::min(minBounds.z, objMin.z);
        
        maxBounds.x = std::max(maxBounds.x, objMax.x);
        maxBounds.y = std::max(maxBounds.y, objMax.y);
        maxBounds.z = std::max(maxBounds.z, objMax.z);
    }
}

bool SelectionManager::rayIntersectSphere(const Ray& ray, const Vec3& center, float radius, float& distance) {
    Vec3 oc = ray.origin - center;
    float a = dot(ray.direction, ray.direction);
    float b = 2.0f * dot(oc, ray.direction);
    float c = dot(oc, oc) - radius * radius;
    
    float discriminant = b * b - 4.0f * a * c;
    if (discriminant < 0) return false;
    
    float sqrtD = std::sqrt(discriminant);
    float t1 = (-b - sqrtD) / (2.0f * a);
    float t2 = (-b + sqrtD) / (2.0f * a);
    
    distance = (t1 > 0.001f) ? t1 : t2;
    return distance > 0.001f;
}

bool SelectionManager::rayIntersectPlane(const Ray& ray, const Vec3& point, const Vec3& normal, float& distance) {
    float denom = dot(normal, ray.direction);
    if (std::abs(denom) < 0.0001f) return false;
    
    distance = dot(point - ray.origin, normal) / denom;
    return distance > 0.001f;
}

bool SelectionManager::rayIntersectAABB(const Ray& ray, const Vec3& minBounds, const Vec3& maxBounds, float& distance) {
    Vec3 invDir = Vec3{1.0f / ray.direction.x, 1.0f / ray.direction.y, 1.0f / ray.direction.z};
    
    Vec3 t1 = (minBounds - ray.origin) * invDir;
    Vec3 t2 = (maxBounds - ray.origin) * invDir;
    
    Vec3 tMin = Vec3{std::min(t1.x, t2.x), std::min(t1.y, t2.y), std::min(t1.z, t2.z)};
    Vec3 tMax = Vec3{std::max(t1.x, t2.x), std::max(t1.y, t2.y), std::max(t1.z, t2.z)};
    
    float tNear = std::max({tMin.x, tMin.y, tMin.z});
    float tFar = std::min({tMax.x, tMax.y, tMax.z});
    
    if (tNear > tFar || tFar < 0) return false;
    
    distance = tNear > 0 ? tNear : tFar;
    return distance > 0.001f;
}