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
};