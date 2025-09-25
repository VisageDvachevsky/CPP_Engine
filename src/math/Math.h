#pragma once

#include "Vec3.h"
#include "Vec2.h"
#include "Mat4.h"

namespace Math {
    constexpr float PI = 3.14159265358979323846f;
    constexpr float TWO_PI = 2.0f * PI;
    constexpr float HALF_PI = PI * 0.5f;
    
    inline float radians(float degrees) {
        return degrees * PI / 180.0f;
    }
    
    inline float degrees(float radians) {
        return radians * 180.0f / PI;
    }
    
    inline float clamp(float value, float min, float max) {
        return value < min ? min : (value > max ? max : value);
    }
    
    inline float lerp(float a, float b, float t) {
        return a + (b - a) * t;
    }
}