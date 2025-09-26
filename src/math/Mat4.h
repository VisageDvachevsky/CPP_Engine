#pragma once

#include "Vec3.h"
#include <cmath>

struct Mat4 {
    float m[16];
    
    Mat4() {
        identity();
    }
    
    Mat4(float diagonal) {
        for (int i = 0; i < 16; ++i) m[i] = 0.0f;
        m[0] = m[5] = m[10] = m[15] = diagonal;
    }
    
    void identity() {
        for (int i = 0; i < 16; ++i) m[i] = 0.0f;
        m[0] = m[5] = m[10] = m[15] = 1.0f;
    }
    
    float* data() { return m; }
    const float* data() const { return m; }
    
    Mat4 operator*(const Mat4& other) const {
        Mat4 result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.m[i * 4 + j] = 0;
                for (int k = 0; k < 4; ++k) {
                    result.m[i * 4 + j] += m[i * 4 + k] * other.m[k * 4 + j];
                }
            }
        }
        return result;
    }
    
    // Matrix inversion using adjugate method
    Mat4 inverse() const {
        // Calculate cofactors and determinant
        float c00 = m[5] * (m[10] * m[15] - m[11] * m[14]) - m[9] * (m[6] * m[15] - m[7] * m[14]) + m[13] * (m[6] * m[11] - m[7] * m[10]);
        float c01 = -(m[1] * (m[10] * m[15] - m[11] * m[14]) - m[9] * (m[2] * m[15] - m[3] * m[14]) + m[13] * (m[2] * m[11] - m[3] * m[10]));
        float c02 = m[1] * (m[6] * m[15] - m[7] * m[14]) - m[5] * (m[2] * m[15] - m[3] * m[14]) + m[13] * (m[2] * m[7] - m[3] * m[6]);
        float c03 = -(m[1] * (m[6] * m[11] - m[7] * m[10]) - m[5] * (m[2] * m[11] - m[3] * m[10]) + m[9] * (m[2] * m[7] - m[3] * m[6]));
        
        float c10 = -(m[4] * (m[10] * m[15] - m[11] * m[14]) - m[8] * (m[6] * m[15] - m[7] * m[14]) + m[12] * (m[6] * m[11] - m[7] * m[10]));
        float c11 = m[0] * (m[10] * m[15] - m[11] * m[14]) - m[8] * (m[2] * m[15] - m[3] * m[14]) + m[12] * (m[2] * m[11] - m[3] * m[10]);
        float c12 = -(m[0] * (m[6] * m[15] - m[7] * m[14]) - m[4] * (m[2] * m[15] - m[3] * m[14]) + m[12] * (m[2] * m[7] - m[3] * m[6]));
        float c13 = m[0] * (m[6] * m[11] - m[7] * m[10]) - m[4] * (m[2] * m[11] - m[3] * m[10]) + m[8] * (m[2] * m[7] - m[3] * m[6]);
        
        float c20 = m[4] * (m[9] * m[15] - m[11] * m[13]) - m[8] * (m[5] * m[15] - m[7] * m[13]) + m[12] * (m[5] * m[11] - m[7] * m[9]);
        float c21 = -(m[0] * (m[9] * m[15] - m[11] * m[13]) - m[8] * (m[1] * m[15] - m[3] * m[13]) + m[12] * (m[1] * m[11] - m[3] * m[9]));
        float c22 = m[0] * (m[5] * m[15] - m[7] * m[13]) - m[4] * (m[1] * m[15] - m[3] * m[13]) + m[12] * (m[1] * m[7] - m[3] * m[5]);
        float c23 = -(m[0] * (m[5] * m[11] - m[7] * m[9]) - m[4] * (m[1] * m[11] - m[3] * m[9]) + m[8] * (m[1] * m[7] - m[3] * m[5]));
        
        float c30 = -(m[4] * (m[9] * m[14] - m[10] * m[13]) - m[8] * (m[5] * m[14] - m[6] * m[13]) + m[12] * (m[5] * m[10] - m[6] * m[9]));
        float c31 = m[0] * (m[9] * m[14] - m[10] * m[13]) - m[8] * (m[1] * m[14] - m[2] * m[13]) + m[12] * (m[1] * m[10] - m[2] * m[9]);
        float c32 = -(m[0] * (m[5] * m[14] - m[6] * m[13]) - m[4] * (m[1] * m[14] - m[2] * m[13]) + m[12] * (m[1] * m[6] - m[2] * m[5]));
        float c33 = m[0] * (m[5] * m[10] - m[6] * m[9]) - m[4] * (m[1] * m[10] - m[2] * m[9]) + m[8] * (m[1] * m[6] - m[2] * m[5]);
        
        // Calculate determinant
        float det = m[0] * c00 + m[4] * c01 + m[8] * c02 + m[12] * c03;
        
        // Check if determinant is zero (non-invertible)
        if (std::abs(det) < 1e-6f) {
            // Return identity matrix if not invertible
            return Mat4(1.0f);
        }
        
        float invDet = 1.0f / det;
        
        // Calculate inverse matrix
        Mat4 result;
        result.m[0] = c00 * invDet;
        result.m[1] = c01 * invDet;
        result.m[2] = c02 * invDet;
        result.m[3] = c03 * invDet;
        
        result.m[4] = c10 * invDet;
        result.m[5] = c11 * invDet;
        result.m[6] = c12 * invDet;
        result.m[7] = c13 * invDet;
        
        result.m[8] = c20 * invDet;
        result.m[9] = c21 * invDet;
        result.m[10] = c22 * invDet;
        result.m[11] = c23 * invDet;
        
        result.m[12] = c30 * invDet;
        result.m[13] = c31 * invDet;
        result.m[14] = c32 * invDet;
        result.m[15] = c33 * invDet;
        
        return result;
    }
    
    // Create a transposed version of the matrix
    Mat4 transpose() const {
        Mat4 result;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                result.m[i * 4 + j] = m[j * 4 + i];
            }
        }
        return result;
    }
    
    static Mat4 perspective(float fov, float aspect, float near, float far) {
        Mat4 result(0);
        float tanHalfFov = std::tan(fov * 0.5f);
        result.m[0] = 1.0f / (aspect * tanHalfFov);
        result.m[5] = 1.0f / tanHalfFov;
        result.m[10] = -(far + near) / (far - near);
        result.m[11] = -1.0f;
        result.m[14] = -(2.0f * far * near) / (far - near);
        return result;
    }
    
    static Mat4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
        Vec3 f = (center - eye).normalized();
        Vec3 s = cross(f, up).normalized();
        Vec3 u = cross(s, f);
        
        Mat4 result;
        result.m[0] = s.x;   result.m[4] = s.y;   result.m[8] = s.z;    result.m[12] = -dot(s, eye);
        result.m[1] = u.x;   result.m[5] = u.y;   result.m[9] = u.z;    result.m[13] = -dot(u, eye);
        result.m[2] = -f.x;  result.m[6] = -f.y;  result.m[10] = -f.z;  result.m[14] = dot(f, eye);
        result.m[3] = 0;     result.m[7] = 0;     result.m[11] = 0;     result.m[15] = 1;
        return result;
    }
    
    static Mat4 translate(const Vec3& v) {
        Mat4 result;
        result.m[12] = v.x;
        result.m[13] = v.y;
        result.m[14] = v.z;
        return result;
    }
    
    static Mat4 scale(const Vec3& v) {
        Mat4 result(0);
        result.m[0] = v.x;
        result.m[5] = v.y;
        result.m[10] = v.z;
        result.m[15] = 1.0f;
        return result;
    }
    
    static Mat4 rotateX(float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        
        Mat4 result;
        result.m[5] = c;
        result.m[6] = -s;
        result.m[9] = s;
        result.m[10] = c;
        
        return result;
    }
    
    static Mat4 rotateY(float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        
        Mat4 result;
        result.m[0] = c;
        result.m[2] = s;
        result.m[8] = -s;
        result.m[10] = c;
        
        return result;
    }
    
    static Mat4 rotateZ(float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        
        Mat4 result;
        result.m[0] = c;
        result.m[1] = -s;
        result.m[4] = s;
        result.m[5] = c;
        
        return result;
    }
};