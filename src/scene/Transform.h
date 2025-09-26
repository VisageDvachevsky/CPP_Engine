#pragma once

#include "math/Vec3.h"
#include "math/Mat4.h"

struct Transform {
    Vec3 position{0, 0, 0};
    Vec3 rotation{0, 0, 0}; // In degrees
    Vec3 scale{1, 1, 1};
    
    Mat4 getMatrix() const;
    Mat4 getInverseMatrix() const;
    
    void setFromMatrix(const Mat4& matrix);
    
    // Helper methods for transforms
    Mat4 getTranslationMatrix() const { return Mat4::translate(position); }
    Mat4 getRotationMatrix() const;
    Mat4 getScaleMatrix() const { return Mat4::scale(scale); }
    
    // Utility methods
    Vec3 getForward() const;
    Vec3 getRight() const;
    Vec3 getUp() const;
    
    // Interpolation for animation
    static Transform lerp(const Transform& a, const Transform& b, float t);
};