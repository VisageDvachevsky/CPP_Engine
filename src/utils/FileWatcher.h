#pragma once

#include <string>
#include <functional>
#include <unordered_map>
#include <filesystem>

class FileWatcher {
public:
    using CallbackType = std::function<void(const std::string&)>;
    
    FileWatcher(const std::string& directory);
    ~FileWatcher() = default;
    
    void update();
    void setCallback(CallbackType callback) { m_callback = callback; }

private:
    std::string m_directory;
    std::unordered_map<std::string, std::filesystem::file_time_type> m_fileTimestamps;
    CallbackType m_callback;
    
    void scanDirectory();
};