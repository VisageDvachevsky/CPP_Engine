#include "StatusBar.h"
#include "renderer/Renderer.h"
#include "core/Time.h"

#include <imgui.h>

void StatusBar::show(const Renderer& renderer) {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse | 
                            ImGuiWindowFlags_NoScrollbar;
    
    if (ImGui::Begin("StatusBar", nullptr, flags)) {
        ImGui::Text("FPS: %.1f (%.3fms)", 
                    Time::getFPS(), 
                    1000.0f / Time::getFPS());
        
        ImGui::SameLine();
        ImGui::Text("|"); 
        ImGui::SameLine();
        
        ImGui::Text("Draw Calls: %d", renderer.getDrawCalls());
        
        ImGui::SameLine();
        ImGui::Text("|"); 
        ImGui::SameLine();
        
        ImGui::Text("Frame: %d", Time::getFrameCount());
        
        ImGui::SameLine();
        ImGui::Text("|"); 
        ImGui::SameLine();
        
        ImGui::Text("SPP: %d | Bounces: %d", 
                    renderer.getSamplesPerPixel(), 
                    renderer.getMaxBounces());
        
        ImGui::SameLine();
        float width = ImGui::GetWindowWidth();
        float textWidth = 200.0f; 
        if (width > textWidth + 50) { 
            ImGui::SetCursorPosX(width - textWidth);
            ImGui::Text("MiniGPU Engine | OpenGL 3.3");
        }
    }
    ImGui::End();
}