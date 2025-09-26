#include "SelectionManager.h"
#include "core/Input.h"
#include "scene/Scene.h"
#include "scene/Object.h"
#include "core/Logger.h"
#include "scene/Camera.h"
#include "renderer/Renderer.h"
#include "math/Ray.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <imgui.h>

SelectionManager::SelectionManager(Scene& scene) : m_scene(scene) {
}

void SelectionManager::update() {
    if (ImGui::GetIO().WantCaptureMouse) {
        return;
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
    if (!object) {
        LOG_WARN("SelectionManager::selectObject called with null object");
        return;
    }
    
    // First deselect all objects
    for (Object* obj : m_selectedObjects) {
        if (obj) {
            obj->setSelected(false);
        }
    }
    
    // Clear list and add new object
    m_selectedObjects.clear();
    m_selectedObjects.push_back(object);
    object->setSelected(true);
    
    // Set selected object in scene (important!)
    m_scene.setSelectedObject(object);
    
    LOG_INFO("Object '{}' selected", object->getName());
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
    LOG_DEBUG("Ray picking, checking {} objects", objects.size());
    
    for (const auto& obj : objects) {
        // Skip invisible objects
        if (!obj->isVisible()) {
            continue;
        }
        
        float distance;
        bool hit = false;
        
        Vec3 position = obj->getTransform().position;
        Vec3 scale = obj->getTransform().scale;
        
        switch (obj->getType()) {
            case ObjectType::Sphere: {
                float radius = scale.x;
                hit = rayIntersectSphere(ray, position, radius, distance);
                break;
            }
            case ObjectType::Plane: {
                Vec3 normal{0, 1, 0};
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
            LOG_DEBUG("Hit object '{}' at distance {:.3f}", obj->getName(), distance);
        }
    }
    
    return closestObject;
}

void SelectionManager::handleMousePicking(const Vec2& mousePos, const Camera& camera) {
    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }

    Ray ray = camera.screenPointToRay(mousePos);
    Object* hitObject = pickObject(ray);
    
    // Get double-click state explicitly - no longer relying on unreliable detection
    bool isDoubleClicked = Input::isMouseButtonDoubleClicked(GLFW_MOUSE_BUTTON_LEFT);
    
    if (hitObject) {
        LOG_INFO("Ray hit object '{}'", hitObject->getName());
        
        // Add debug info
        LOG_DEBUG("Object position: [{:.2f}, {:.2f}, {:.2f}]", 
                 hitObject->getTransform().position.x,
                 hitObject->getTransform().position.y,
                 hitObject->getTransform().position.z);
        
        if (isDoubleClicked) {
            LOG_INFO("Double click detected on object '{}'", hitObject->getName());
            
            // Always select on double click
            selectObject(hitObject);
            
            // Focus camera on double-clicked object
            if (m_objectFocusCallback) {
                Vec3 position = hitObject->getTransform().position;
                float radius = std::max(
                    std::max(hitObject->getTransform().scale.x, 
                             hitObject->getTransform().scale.y),
                             hitObject->getTransform().scale.z);
                LOG_INFO("Focusing camera on object '{}', position=[{:.2f}, {:.2f}, {:.2f}], radius={:.2f}", 
                         hitObject->getName(), position.x, position.y, position.z, radius);
                m_objectFocusCallback(position, radius);
            }
            
            // Activate transform gizmo (this will be picked up by Editor)
            if (m_activateGizmoCallback) {
                LOG_INFO("Activating transform gizmo");
                m_activateGizmoCallback();
            }
        } else {
            // Single click just selects the object
            LOG_INFO("Single click - selecting object '{}'", hitObject->getName());
            selectObject(hitObject);
        }
    } else {
        LOG_INFO("Ray did not hit any object");
        
        // On single click in empty space, deselect all
        if (!isDoubleClicked) {
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
    
    // Use nearest positive intersection
    if (t1 > 0.001f) {
        distance = t1;
        return true;
    }
    
    if (t2 > 0.001f) {
        distance = t2;
        return true;
    }
    
    return false;
}

bool SelectionManager::rayIntersectPlane(const Ray& ray, const Vec3& point, const Vec3& normal, float& distance) {
    float denom = dot(normal, ray.direction);
    if (std::abs(denom) < 0.0001f) return false;
    
    distance = dot(point - ray.origin, normal) / denom;
    return distance > 0.001f;
}

bool SelectionManager::rayIntersectAABB(const Ray& ray, const Vec3& minBounds, const Vec3& maxBounds, float& distance) {
    Vec3 invDir = Vec3{
        ray.direction.x != 0 ? 1.0f / ray.direction.x : std::numeric_limits<float>::max(),
        ray.direction.y != 0 ? 1.0f / ray.direction.y : std::numeric_limits<float>::max(),
        ray.direction.z != 0 ? 1.0f / ray.direction.z : std::numeric_limits<float>::max()
    };
    
    Vec3 t1 = (minBounds - ray.origin) * invDir;
    Vec3 t2 = (maxBounds - ray.origin) * invDir;
    
    Vec3 tMin = Vec3{
        std::min(t1.x, t2.x), 
        std::min(t1.y, t2.y), 
        std::min(t1.z, t2.z)
    };
    Vec3 tMax = Vec3{
        std::max(t1.x, t2.x), 
        std::max(t1.y, t2.y), 
        std::max(t1.z, t2.z)
    };
    
    float tNear = std::max(std::max(tMin.x, tMin.y), tMin.z);
    float tFar = std::min(std::min(tMax.x, tMax.y), tMax.z);
    
    // Check if there's a valid intersection
    if (tNear > tFar || tFar < 0) {
        return false;
    }
    
    // Return the nearest positive intersection point
    distance = tNear > 0.001f ? tNear : tFar;
    return distance > 0.001f;
}

