#pragma once

#include "renderer/Shader.h"
#include <memory>
#include <unordered_map>
#include <string>

class ResourceManager {
public:
    static ResourceManager& instance();
    
    std::shared_ptr<Shader> loadShader(const std::string& name, 
                                      const std::string& vertexPath, 
                                      const std::string& fragmentPath);
    
    std::shared_ptr<Shader> getShader(const std::string& name);
    void clearShaders();
    
    // Texture management would go here
    // unsigned int loadTexture(const std::string& path);

private:
    ResourceManager() = default;
    
    std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;
};