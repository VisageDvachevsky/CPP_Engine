#include "FileWatcher.h"
#include "core/Logger.h"

#include <filesystem>

FileWatcher::FileWatcher(const std::string& directory) 
    : m_directory(directory) {
    scanDirectory();
    LOG_INFO("FileWatcher monitoring: {}", directory);
}

void FileWatcher::update() {
    try {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(m_directory)) {
            if (!entry.is_regular_file()) continue;
            
            std::string path = entry.path().string();
            auto lastWriteTime = entry.last_write_time();
            
            auto it = m_fileTimestamps.find(path);
            if (it == m_fileTimestamps.end()) {
                // New file
                m_fileTimestamps[path] = lastWriteTime;
            } else if (it->second != lastWriteTime) {
                // File modified
                it->second = lastWriteTime;
                
                if (m_callback) {
                    m_callback(path);
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        LOG_ERROR("FileWatcher error: {}", e.what());
    }
}

void FileWatcher::scanDirectory() {
    try {
        m_fileTimestamps.clear();
        
        for (const auto& entry : std::filesystem::recursive_directory_iterator(m_directory)) {
            if (entry.is_regular_file()) {
                m_fileTimestamps[entry.path().string()] = entry.last_write_time();
            }
        }
        
        LOG_DEBUG("FileWatcher scanned {} files", m_fileTimestamps.size());
    } catch (const std::filesystem::filesystem_error& e) {
        LOG_ERROR("FileWatcher scan error: {}", e.what());
    }
}