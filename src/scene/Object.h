#pragma once

#include "Material.h"
#include "math/Vec3.h"
#include <string>

enum class ObjectType {
    Sphere,
    Plane,
    Cube
};

struct Transform {
    Vec3 position{0, 0, 0};
    Vec3 rotation{0, 0, 0};
    Vec3 scale{1, 1, 1};
};

class Object {
public:
    Object(const std::string& name, ObjectType type);
    ~Object() = default;
    
    void update(float dt);
    
    const std::string& getName() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }
    
    ObjectType getType() const { return m_type; }
    
    Transform& getTransform() { return m_transform; }
    const Transform& getTransform() const { return m_transform; }
    
    Material& getMaterial() { return m_material; }
    const Material& getMaterial() const { return m_material; }
    
    Vec3 getScale() const { return m_transform.scale; }
    void setScale(const Vec3& scale) { m_transform.scale = scale; }
    
    bool isSelected() const { return m_selected; }
    void setSelected(bool selected) { m_selected = selected; }

private:
    std::string m_name;
    ObjectType m_type;
    Transform m_transform;
    Material m_material;
    bool m_selected = false;
};