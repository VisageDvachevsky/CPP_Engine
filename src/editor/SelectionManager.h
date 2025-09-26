// src/editor/SelectionManager.h
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

// Определяем тип колбэка для фокусировки на объекте
using ObjectFocusCallback = std::function<void(const Vec3& position, float radius)>;

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
    
    // Метод для установки колбэка фокусировки
    void setObjectFocusCallback(const ObjectFocusCallback& callback) { m_objectFocusCallback = callback; }
    
    // Методы выбора
    void selectObject(Object* object);
    void deselectAll();
    void addToSelection(Object* object);
    void removeFromSelection(Object* object);
    
    // Запросы
    bool hasSelection() const { return !m_selectedObjects.empty(); }
    Object* getSelectedObject() const;
    const std::vector<Object*>& getSelectedObjects() const { return m_selectedObjects; }
    
    // Выбор мышью
    Object* pickObject(const Ray& ray);
    void handleMousePicking(const Vec2& mousePos, const Camera& camera);
    
    // Вычисление границ
    void getSelectionBounds(Vec3& minBounds, Vec3& maxBounds) const;

private:
    bool rayIntersectSphere(const Ray& ray, const Vec3& center, float radius, float& distance);
    bool rayIntersectPlane(const Ray& ray, const Vec3& point, const Vec3& normal, float& distance);
    bool rayIntersectAABB(const Ray& ray, const Vec3& minBounds, const Vec3& maxBounds, float& distance);
    
    Scene& m_scene;
    std::vector<Object*> m_selectedObjects;
    Object* m_hoveredObject = nullptr;
    
    // Колбэк для фокусировки камеры на объекте
    ObjectFocusCallback m_objectFocusCallback;
};