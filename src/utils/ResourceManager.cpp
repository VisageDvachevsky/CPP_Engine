#include "ResourceManager.h"
#include "core/Logger.h"

ResourceManager& ResourceManager::instance() {
    static ResourceManager instance;
    return instance;
}

std::shared_ptr<Shader> ResourceManager::loadShader(const std::string& name, 
                                                   const std::string& vertexPath, 
                                                   const std::string& fragmentPath) {
    auto it = m_shaders.find(name);
    if (it != m_shaders.end()) {
        LOG_DEBUG("Shader '{}' already loaded, returning cached version", name);
        return it->second;
    }
    
    auto shader = std::make_shared<Shader>();
    if (shader->loadFromFiles(vertexPath, fragmentPath)) {
        m_shaders[name] = shader;
        LOG_INFO("Shader '{}' loaded successfully", name);
        return shader;
    }
    
    LOG_ERROR("Failed to load shader '{}'", name);
    return nullptr;
}

std::shared_ptr<Shader> ResourceManager::getShader(const std::string& name) {
    auto it = m_shaders.find(name);
    if (it != m_shaders.end()) {
        return it->second;
    }
    
    LOG_WARN("Shader '{}' not found", name);
    return nullptr;
}

void ResourceManager::clearShaders() {
    LOG_INFO("Clearing {} cached shaders", m_shaders.size());
    m_shaders.clear();
}