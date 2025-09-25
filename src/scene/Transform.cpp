#include "Transform.h"
#include <cmath>

Mat4 Transform::getMatrix() const {
    Mat4 translation = Mat4::translate(position);
    
    float yawRad = rotation.y * (3.14159f / 180.0f);
    Mat4 rotationY;
    rotationY.m[0] = cos(yawRad);  rotationY.m[2] = sin(yawRad);
    rotationY.m[8] = -sin(yawRad); rotationY.m[10] = cos(yawRad);
    
    Mat4 scaleMatrix = Mat4::scale(scale);
    
    return translation * rotationY * scaleMatrix;
}

Mat4 Transform::getInverseMatrix() const {
    Mat4 invScale = Mat4::scale(Vec3{1.0f/scale.x, 1.0f/scale.y, 1.0f/scale.z});
    
    float yawRad = -rotation.y * (3.14159f / 180.0f); // Negative for inverse
    Mat4 invRotationY;
    invRotationY.m[0] = cos(yawRad);  invRotationY.m[2] = sin(yawRad);
    invRotationY.m[8] = -sin(yawRad); invRotationY.m[10] = cos(yawRad);
    
    Mat4 invTranslation = Mat4::translate(-position);
    
    return invScale * invRotationY * invTranslation;
}

void Transform::setFromMatrix(const Mat4& matrix) {
    // Extract position (translation)
    position.x = matrix.m[12];
    position.y = matrix.m[13]; 
    position.z = matrix.m[14];
    
    // Extract scale
    Vec3 col0{matrix.m[0], matrix.m[1], matrix.m[2]};
    Vec3 col1{matrix.m[4], matrix.m[5], matrix.m[6]};
    Vec3 col2{matrix.m[8], matrix.m[9], matrix.m[10]};
    
    scale.x = col0.length();
    scale.y = col1.length();
    scale.z = col2.length();
    
    // Extract rotation 
    if (scale.x > 0.001f && scale.z > 0.001f) {
        float normalizedM00 = matrix.m[0] / scale.x;
        float normalizedM02 = matrix.m[2] / scale.x;
        rotation.y = atan2(normalizedM02, normalizedM00) * (180.0f / 3.14159f);
    }
}