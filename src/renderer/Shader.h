#pragma once

#include "math/Vec3.h"
#include "math/Vec2.h"
#include "math/Mat4.h"
#include <string>
#include <unordered_map>

class Shader {
public:
    Shader() = default;
    ~Shader();
    
    bool loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath);
    bool loadFromString(const std::string& vertexSource, const std::string& fragmentSource);
    
    void use() const;
    void unuse() const;
    
    void setInt(const std::string& name, int value);
    void setFloat(const std::string& name, float value);
    void setVec3(const std::string& name, const Vec3& value);
    void setMat4(const std::string& name, const Mat4& value);
    void setVec2(const std::string& name, const Vec2& value);
    
    unsigned int getID() const { return m_program; }
    bool isValid() const { return m_program != 0; }

private:
    unsigned int compileShader(const std::string& source, unsigned int type);
    bool linkProgram(unsigned int vertex, unsigned int fragment);
    int getUniformLocation(const std::string& name);
    
    unsigned int m_program = 0;
    mutable std::unordered_map<std::string, int> m_uniformCache;
};