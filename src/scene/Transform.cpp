#include "Transform.h"
#include <cmath>

Mat4 Transform::getMatrix() const {
    Mat4 translation = Mat4::translate(position);
    Mat4 rotationMatrix = getRotationMatrix();
    Mat4 scaleMatrix = Mat4::scale(scale);
    
    return translation * rotationMatrix * scaleMatrix;
}

Mat4 Transform::getInverseMatrix() const {
    Mat4 invScale = Mat4::scale(Vec3{1.0f/scale.x, 1.0f/scale.y, 1.0f/scale.z});
    Mat4 invRotation = getRotationMatrix().inverse();
    Mat4 invTranslation = Mat4::translate(-position);
    
    return invScale * invRotation * invTranslation;
}

Mat4 Transform::getRotationMatrix() const {
    // Convert Euler angles to radians
    float yawRad = rotation.y * (M_PI / 180.0f);
    float pitchRad = rotation.x * (M_PI / 180.0f);
    float rollRad = rotation.z * (M_PI / 180.0f);
    
    Mat4 rotationX, rotationY, rotationZ;
    
    // Rotation around X axis (pitch)
    rotationX = Mat4(1.0f);
    rotationX.m[5] = cos(pitchRad);
    rotationX.m[6] = -sin(pitchRad);
    rotationX.m[9] = sin(pitchRad);
    rotationX.m[10] = cos(pitchRad);
    
    // Rotation around Y axis (yaw)
    rotationY = Mat4(1.0f);
    rotationY.m[0] = cos(yawRad);
    rotationY.m[2] = sin(yawRad);
    rotationY.m[8] = -sin(yawRad);
    rotationY.m[10] = cos(yawRad);
    
    // Rotation around Z axis (roll)
    rotationZ = Mat4(1.0f);
    rotationZ.m[0] = cos(rollRad);
    rotationZ.m[1] = -sin(rollRad);
    rotationZ.m[4] = sin(rollRad);
    rotationZ.m[5] = cos(rollRad);
    
    // Combine rotations: Y * X * Z (YXZ order)
    return rotationY * rotationX * rotationZ;
}

void Transform::setFromMatrix(const Mat4& matrix) {
    // Extract position
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
    // First normalize the matrix to remove scale
    Mat4 rotMatrix = matrix;
    
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            rotMatrix.m[i*4+j] /= scale[i];
        }
    }
    
    // Extract Euler angles (in degrees)
    // Assuming YXZ rotation order
    float sy = sqrt(rotMatrix.m[0]*rotMatrix.m[0] + rotMatrix.m[4]*rotMatrix.m[4]);
    
    bool singular = sy < 1e-6;
    
    if (!singular) {
        rotation.x = atan2(rotMatrix.m[9], rotMatrix.m[10]) * (180.0f/M_PI);
        rotation.y = atan2(-rotMatrix.m[8], sy) * (180.0f/M_PI);
        rotation.z = atan2(rotMatrix.m[4], rotMatrix.m[0]) * (180.0f/M_PI);
    } else {
        rotation.x = atan2(-rotMatrix.m[6], rotMatrix.m[5]) * (180.0f/M_PI);
        rotation.y = atan2(-rotMatrix.m[8], sy) * (180.0f/M_PI);
        rotation.z = 0.0f;
    }
}

Vec3 Transform::getForward() const {
    // Calculate forward vector from rotation (typically -Z in OpenGL)
    float yawRad = rotation.y * (M_PI / 180.0f);
    float pitchRad = rotation.x * (M_PI / 180.0f);
    
    Vec3 forward;
    forward.x = sin(yawRad) * cos(pitchRad);
    forward.y = sin(pitchRad);
    forward.z = cos(yawRad) * cos(pitchRad);
    
    return forward.normalized();
}

Vec3 Transform::getRight() const {
    // Right vector is perpendicular to forward and world-up
    Vec3 worldUp{0.0f, 1.0f, 0.0f};
    Vec3 forward = getForward();
    
    return cross(worldUp, forward).normalized();
}

Vec3 Transform::getUp() const {
    // Up vector completes the orthogonal basis
    Vec3 forward = getForward();
    Vec3 right = getRight();
    
    return cross(forward, right).normalized();
}

Transform Transform::lerp(const Transform& a, const Transform& b, float t) {
    Transform result;
    
    // Linearly interpolate position
    result.position = a.position + (b.position - a.position) * t;
    
    // Linearly interpolate scale
    result.scale = a.scale + (b.scale - a.scale) * t;
    
    // Interpolate rotation (simple Euler angle interpolation)
    // Note: For better results, quaternions would be used, but we're keeping it simple
    Vec3 rotDiff = b.rotation - a.rotation;
    
    // Handle angle wrapping (keep rotations within -180 to 180 range)
    for (int i = 0; i < 3; i++) {
        if (rotDiff[i] > 180.0f) rotDiff[i] -= 360.0f;
        else if (rotDiff[i] < -180.0f) rotDiff[i] += 360.0f;
    }
    
    result.rotation = a.rotation + rotDiff * t;
    
    return result;
}