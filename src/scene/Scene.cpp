#include "Scene.h"
#include "Material.h"
#include "core/Logger.h"

Scene::Scene() {
    LOG_INFO("Scene created");
}

void Scene::update(float dt) {
    for (auto& object : m_objects) {
        object->update(dt);
    }
}

void Scene::createDefaultScene() {
    LOG_INFO("Creating default scene...");
    
    // Ground plane
    auto ground = std::make_unique<Object>("Ground", ObjectType::Plane);
    ground->getTransform().position = {0, 0, 0};
    ground->setScale({10, 1, 10});
    ground->getMaterial().color = {0.5f, 0.5f, 0.5f};
    ground->getMaterial().type = MaterialType::Diffuse;
    addObject(std::move(ground));
    
    // Center sphere - metal
    auto centerSphere = std::make_unique<Object>("Metal Sphere", ObjectType::Sphere);
    centerSphere->getTransform().position = {0, 1, 0};
    centerSphere->setScale({1, 1, 1});
    centerSphere->getMaterial().color = {0.7f, 0.6f, 0.5f};
    centerSphere->getMaterial().type = MaterialType::Metal;
    centerSphere->getMaterial().roughness = 0.0f;
    addObject(std::move(centerSphere));
    
    // Left sphere - diffuse
    auto leftSphere = std::make_unique<Object>("Diffuse Sphere", ObjectType::Sphere);
    leftSphere->getTransform().position = {-2, 1, 0};
    leftSphere->setScale({1, 1, 1});
    leftSphere->getMaterial().color = {0.1f, 0.2f, 0.5f};
    leftSphere->getMaterial().type = MaterialType::Diffuse;
    addObject(std::move(leftSphere));
    
    // Right sphere - glass
    auto rightSphere = std::make_unique<Object>("Glass Sphere", ObjectType::Sphere);
    rightSphere->getTransform().position = {2, 1, 0};
    rightSphere->setScale({1, 1, 1});
    rightSphere->getMaterial().color = {1.0f, 1.0f, 1.0f};
    rightSphere->getMaterial().type = MaterialType::Dielectric;
    rightSphere->getMaterial().ior = 1.5f;
    addObject(std::move(rightSphere));
    
    // Small sphere - emissive-like bright diffuse
    auto smallSphere = std::make_unique<Object>("Bright Sphere", ObjectType::Sphere);
    smallSphere->getTransform().position = {0, 3, 1};
    smallSphere->setScale({0.5f, 0.5f, 0.5f});
    smallSphere->getMaterial().color = {4.0f, 2.0f, 1.0f}; // Bright for lighting effect
    smallSphere->getMaterial().type = MaterialType::Diffuse;
    addObject(std::move(smallSphere));
    
    LOG_INFO("Default scene created with {} objects", m_objects.size());
}

void Scene::addObject(std::unique_ptr<Object> object) {
    m_objects.push_back(std::move(object));
}

void Scene::removeObject(int index) {
    if (index >= 0 && index < static_cast<int>(m_objects.size())) {
        if (m_objects[index].get() == m_selectedObject) {
            m_selectedObject = nullptr;
        }
        m_objects.erase(m_objects.begin() + index);
    }
}

int Scene::getSelectedIndex() const {
    if (!m_selectedObject) return -1;
    
    for (int i = 0; i < static_cast<int>(m_objects.size()); ++i) {
        if (m_objects[i].get() == m_selectedObject) {
            return i;
        }
    }
    return -1;
}