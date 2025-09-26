#include "LogWindow.h"
#include "core/Logger.h"

#include <imgui.h>
#include <chrono>
#include <iomanip>
#include <sstream>

void LogWindow::show() {
    ImGui::Begin("Logger");
    
    ImGui::Checkbox("Auto-scroll", &m_autoScroll);
    ImGui::SameLine();
    
    if (ImGui::Button("Clear")) {
        Logger::clear();
    }
    ImGui::SameLine();

    if (ImGui::Button("Test Double Click")) {
        LOG_INFO("Manual double click test triggered");
        // Эта кнопка только для отладки - она позволит проверить, работает ли обработка двойного клика в целом
    }
    
    // Log level filter
    const char* levels[] = {"All", "Info+", "Warn+", "Error"};
    ImGui::Combo("Level", &m_selectedLevel, levels, 4);
    
    ImGui::Separator();
    
    if (ImGui::BeginChild("LogEntries")) {
        const auto& entries = Logger::getEntries();
        
        for (const auto& entry : entries) {
            int entryLevel = static_cast<int>(entry.level);
            if (m_selectedLevel > 0 && entryLevel < m_selectedLevel) {
                continue;
            }
            
            auto time_t = std::chrono::system_clock::to_time_t(entry.timestamp);
            std::stringstream ss;
            ss << std::put_time(std::localtime(&time_t), "%H:%M:%S");
            
            ImVec4 color;
            const char* levelStr;
            switch (entry.level) {
                case LogLevel::DEBUG:
                    color = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);
                    levelStr = "DEBUG";
                    break;
                case LogLevel::INFO:
                    color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                    levelStr = "INFO ";
                    break;
                case LogLevel::WARN:
                    color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
                    levelStr = "WARN ";
                    break;
                case LogLevel::ERROR:
                    color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
                    levelStr = "ERROR";
                    break;
            }
            
            ImGui::TextColored(color, "[%s] %s: %s", 
                ss.str().c_str(), levelStr, entry.message.c_str());
        }
        
        if (m_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
        }
    }
    ImGui::EndChild();
    
    ImGui::End();
}