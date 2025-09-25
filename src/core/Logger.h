#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <format>

enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3
};

struct LogEntry {
    std::chrono::system_clock::time_point timestamp;
    LogLevel level;
    std::string message;
};

class Logger {
public:
    static void init();
    static void log(LogLevel level, const std::string& message);
    static const std::vector<LogEntry>& getEntries() { return s_entries; }
    static void clear() { s_entries.clear(); }
    static void setLevel(LogLevel level) { s_minLevel = level; }
    
    template<typename... Args>
    static void log(LogLevel level, const std::string& fmt, Args&&... args) {
        if (level >= s_minLevel) {
            log(level, std::vformat(fmt, std::make_format_args(args...)));
        }
    }

private:
    static std::vector<LogEntry> s_entries;
    static LogLevel s_minLevel;
};

#define LOG_DEBUG(...) Logger::log(LogLevel::DEBUG, __VA_ARGS__)
#define LOG_INFO(...) Logger::log(LogLevel::INFO, __VA_ARGS__)
#define LOG_WARN(...) Logger::log(LogLevel::WARN, __VA_ARGS__)
#define LOG_ERROR(...) Logger::log(LogLevel::ERROR, __VA_ARGS__)