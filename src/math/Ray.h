#pragma once

#include "Vec3.h"

struct Ray {
    Vec3 origin;
    Vec3 direction;
    
    Ray() = default;
    Ray(const Vec3& orig, const Vec3& dir) : origin(orig), direction(dir) {}
    
    Vec3 at(float t) const {
        return origin + direction * t;
    }
};