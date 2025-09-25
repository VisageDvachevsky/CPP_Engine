#pragma once

#include "math/Vec3.h"

enum class MaterialType {
    Diffuse = 0,
    Metal = 1,
    Dielectric = 2
};

struct Material {
    Vec3 color{0.7f, 0.3f, 0.3f};
    MaterialType type = MaterialType::Diffuse;
    float roughness = 0.5f;
    float metalness = 0.0f;
    float ior = 1.5f; 
    
    Material() = default;
    
    Material(const Vec3& col, MaterialType t = MaterialType::Diffuse) 
        : color(col), type(t) {}
};