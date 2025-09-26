#pragma once

#include "Material.h"
#include "Transform.h"
#include <string>
#include <memory>

enum class ObjectType {
    Sphere,
    Plane,
    Cube,
    Mesh  // For future expansion
};

// Forward declarations
class ShaderProgram;
class Renderer;

class Object {
public:
    Object(const std::string& name, ObjectType type);
    virtual ~Object() = default;
    
    virtual void update(float dt);
    virtual void render(Renderer& renderer) const;
    
    // Geometry data access for ray tracing
    virtual void getIntersectionData(struct IntersectionData& data) const;
    
    // Accessors
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
    
    // For editor visibility
    bool isVisible() const { return m_visible; }
    void setVisible(bool visible) { m_visible = visible; }

private:
    std::string m_name;
    ObjectType m_type;
    Transform m_transform;
    Material m_material;
    bool m_selected = false;
    bool m_visible = true;
};

// Structure for passing primitive data to shaders/ray tracing
struct IntersectionData {
    ObjectType type;
    Vec3 position;
    Vec3 scale;
    Vec3 color;
    int materialType;
    float roughness;
    float ior;
    float metalness;
    Vec3 emission;
    Vec3 normal; // For planes
    
    // For meshes
    unsigned int vertexCount = 0;
    const float* vertices = nullptr;
    unsigned int indexCount = 0;
    const unsigned int* indices = nullptr;
};