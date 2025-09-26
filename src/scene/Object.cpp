#include "Object.h"
#include "renderer/Renderer.h"
#include "core/Logger.h"

Object::Object(const std::string& name, ObjectType type) 
    : m_name(name), m_type(type) {
}

void Object::update(float dt) {
    // Basic object updates - may be extended in derived classes
}

void Object::render(Renderer& renderer) const {
    // Default implementation - may be overridden in derived classes
}

void Object::getIntersectionData(IntersectionData& data) const {
    // Fill common data
    data.type = m_type;
    data.position = m_transform.position;
    data.scale = m_transform.scale;
    data.color = m_material.color;
    data.materialType = static_cast<int>(m_material.type);
    data.roughness = m_material.roughness;
    data.ior = m_material.ior;
    data.metalness = m_material.metalness;
    data.emission = m_material.emission;
    
    // Type-specific data
    switch (m_type) {
        case ObjectType::Sphere:
            // For spheres, the scale.x is treated as radius
            break;
            
        case ObjectType::Plane:
            // Default plane normal is up (Y+)
            data.normal = Vec3{0.0f, 1.0f, 0.0f};
            // Apply rotation if needed
            if (m_transform.rotation != Vec3{0, 0, 0}) {
                // Get rotation matrix and transform the normal
                Mat4 rotMatrix = m_transform.getRotationMatrix();
                Vec3 defaultNormal{0, 1, 0};
                
                // Apply rotation matrix to transform the normal
                Vec3 tmp = {
                    rotMatrix.m[0] * defaultNormal.x + rotMatrix.m[4] * defaultNormal.y + rotMatrix.m[8] * defaultNormal.z,
                    rotMatrix.m[1] * defaultNormal.x + rotMatrix.m[5] * defaultNormal.y + rotMatrix.m[9] * defaultNormal.z,
                    rotMatrix.m[2] * defaultNormal.x + rotMatrix.m[6] * defaultNormal.y + rotMatrix.m[10] * defaultNormal.z
                };
                
                data.normal = tmp;
            }
            break;
            
        case ObjectType::Cube:
            // For cubes, we use AABB intersection in the shader
            break;
            
        default:
            LOG_WARN("Unknown object type in getIntersectionData");
            break;
    }
}