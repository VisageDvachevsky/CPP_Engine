#pragma once

#include "math/Vec3.h"
#include "math/Mat4.h"

struct Transform {
    Vec3 position{0, 0, 0};
    Vec3 rotation{0, 0, 0};
    Vec3 scale{1, 1, 1};
    
    Mat4 getMatrix() const;
    Mat4 getInverseMatrix() const;
    
    void setFromMatrix(const Mat4& matrix);
};