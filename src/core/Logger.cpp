#include "Logger.h"
#include <iostream>

std::vector<LogEntry> Logger::s_entries;
LogLevel Logger::s_minLevel = LogLevel::INFO;

void Logger::init() {
    s_entries.reserve(1000);
    log(LogLevel::INFO, "Logger initialized");
}

void Logger::log(LogLevel level, const std::string& message) {
    if (level < s_minLevel) return;
    
    LogEntry entry{
        std::chrono::system_clock::now(),
        level,
        message
    };
    
    s_entries.push_back(entry);
    
    // Console output
    const char* levelStr[] = {"DEBUG", "INFO", "WARN", "ERROR"};
    std::cout << "[" << levelStr[static_cast<int>(level)] << "] " << message << std::endl;
    
    // Keep only last 1000 entries
    if (s_entries.size() > 1000) {
        s_entries.erase(s_entries.begin(), s_entries.begin() + 100);
    }
}