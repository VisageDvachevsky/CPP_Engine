#pragma once

#include "Object.h"
#include <vector>
#include <memory>

class Scene {
public:
    Scene();
    ~Scene() = default;
    
    void update(float dt);
    void createDefaultScene();
    
    void addObject(std::unique_ptr<Object> object);
    void removeObject(int index);
    
    const std::vector<std::unique_ptr<Object>>& getObjects() const { return m_objects; }
    std::vector<std::unique_ptr<Object>>& getObjects() { return m_objects; }
    
    Object* getSelectedObject() const { return m_selectedObject; }
    void setSelectedObject(Object* object) { m_selectedObject = object; }
    int getSelectedIndex() const;

private:
    std::vector<std::unique_ptr<Object>> m_objects;
    Object* m_selectedObject = nullptr;
};