#pragma once

#include "Object.h"
#include <vector>
#include <memory>
#include <string>

class Scene {
public:
    Scene();
    ~Scene() = default;
    
    void update(float dt);
    
    // Scene setup
    void createDefaultScene();
    void clear();
    
    // Object management
    void addObject(std::unique_ptr<Object> object);
    void removeObject(int index);
    
    // Object access
    const std::vector<std::unique_ptr<Object>>& getObjects() const { return m_objects; }
    std::vector<std::unique_ptr<Object>>& getObjects() { return m_objects; }
    
    int getObjectCount() const { return static_cast<int>(m_objects.size()); }
    int getObjectIndex(const Object* object) const;
    Object* getObjectByName(const std::string& name);
    
    // Selection management
    Object* getSelectedObject() const { return m_selectedObject; }
    void setSelectedObject(Object* object) { m_selectedObject = object; }
    int getSelectedIndex() const;
    
    // Serialization
    void saveToDisk(const std::string& filePath);
    bool loadFromDisk(const std::string& filePath);
    
    // Factory methods
    static std::unique_ptr<Scene> createEmptyScene();

private:
    std::vector<std::unique_ptr<Object>> m_objects;
    Object* m_selectedObject = nullptr;
};