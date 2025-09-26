#include "Shader.h"
#include "core/Logger.h"
#include <glad/glad.h>
#include <fstream>
#include <sstream>

Shader::~Shader() {
    if (m_program) {
        glDeleteProgram(m_program);
    }
}

bool Shader::loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath) {
    std::ifstream vShaderFile, fShaderFile;
    std::string vertexCode, fragmentCode;
    
    try {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        
        std::stringstream vShaderStream, fShaderStream;
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        
        vShaderFile.close();
        fShaderFile.close();
        
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure& e) {
        LOG_ERROR("Failed to read shader files: {} {}", vertexPath, fragmentPath);
        return false;
    }
    
    return loadFromString(vertexCode, fragmentCode);
}

bool Shader::loadFromString(const std::string& vertexSource, const std::string& fragmentSource) {
    if (m_program) {
        glDeleteProgram(m_program);
        m_program = 0;
        m_uniformCache.clear();
    }
    
    unsigned int vertex = compileShader(vertexSource, GL_VERTEX_SHADER);
    if (!vertex) return false;
    
    unsigned int fragment = compileShader(fragmentSource, GL_FRAGMENT_SHADER);
    if (!fragment) {
        glDeleteShader(vertex);
        return false;
    }
    
    bool success = linkProgram(vertex, fragment);
    
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    
    return success;
}

void Shader::use() const {
    if (m_program) {
        glUseProgram(m_program);
    }
}

void Shader::unuse() const {
    glUseProgram(0);
}

unsigned int Shader::compileShader(const std::string& source, unsigned int type) {
    unsigned int shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        const char* typeStr = (type == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";
        LOG_ERROR("Shader compilation failed ({}): {}", typeStr, infoLog);
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

bool Shader::linkProgram(unsigned int vertex, unsigned int fragment) {
    m_program = glCreateProgram();
    glAttachShader(m_program, vertex);
    glAttachShader(m_program, fragment);
    glLinkProgram(m_program);
    
    int success;
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_program, 512, nullptr, infoLog);
        LOG_ERROR("Shader linking failed: {}", infoLog);
        glDeleteProgram(m_program);
        m_program = 0;
        return false;
    }
    
    return true;
}

int Shader::getUniformLocation(const std::string& name) {
    if (m_program == 0) {
        return -1;  
    }
    
    if (m_uniformCache.find(name) != m_uniformCache.end()) {
        return m_uniformCache[name];
    }
    
    int location = glGetUniformLocation(m_program, name.c_str());
    m_uniformCache[name] = location;
    return location;
}

void Shader::setInt(const std::string& name, int value) {
    if (m_program == 0) {
        LOG_WARN("Attempting to set uniform '{}' on invalid shader", name);
        return;
    }
    
    int location = getUniformLocation(name);
    if (location == -1) {
        LOG_DEBUG("Uniform '{}' not found in shader", name);
        return;
    }
    
    glUniform1i(location, value);
}

void Shader::setFloat(const std::string& name, float value) {
    if (m_program == 0) {
        LOG_WARN("Attempting to set uniform '{}' on invalid shader", name);
        return;
    }
    
    int location = getUniformLocation(name);
    if (location == -1) {
        LOG_DEBUG("Uniform '{}' not found in shader", name);
        return;
    }
    
    glUniform1f(location, value);
}

void Shader::setVec3(const std::string& name, const Vec3& value) {
    if (m_program == 0) {
        LOG_WARN("Attempting to set uniform '{}' on invalid shader", name);
        return;
    }
    
    int location = getUniformLocation(name);
    if (location == -1) {
        LOG_DEBUG("Uniform '{}' not found in shader", name);
        return;
    }
    
    glUniform3fv(location, 1, value.data());
}

void Shader::setMat4(const std::string& name, const Mat4& value) {
    if (m_program == 0) {
        LOG_WARN("Attempting to set uniform '{}' on invalid shader", name);
        return;
    }
    
    int location = getUniformLocation(name);
    if (location == -1) {
        LOG_DEBUG("Uniform '{}' not found in shader", name);
        return;
    }
    
    glUniformMatrix4fv(location, 1, GL_FALSE, value.data());
}

void Shader::setVec2(const std::string& name, const Vec2& value) {
    if (m_program == 0) {
        LOG_WARN("Attempting to set uniform '{}' on invalid shader", name);
        return;
    }
    
    int location = getUniformLocation(name);
    if (location == -1) {
        LOG_DEBUG("Uniform '{}' not found in shader", name);
        return;
    }
    
    glUniform2fv(location, 1, value.data());
}