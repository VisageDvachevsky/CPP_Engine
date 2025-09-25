#include "StatusBar.h"
#include "renderer/Renderer.h"
#include "core/Time.h"

#include <imgui.h>

void StatusBar::show(const Renderer& renderer) {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | 
                            ImGuiWindowFlags_NoMove | 
                            ImGuiWindowFlags_NoResize |
                            ImGuiWindowFlags_NoSavedSettings |
                            ImGuiWindowFlags_NoScrollbar;
    
    ImGui::Begin("StatusBar", nullptr, flags);
    
    // Performance stats
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
    
    // Render settings
    ImGui::Text("SPP: %d | Bounces: %d", 
                renderer.getSamplesPerPixel(), 
                renderer.getMaxBounces());
    
    // Right-aligned content
    ImGui::SameLine();
    float width = ImGui::GetWindowWidth();
    float textWidth = 200.0f; // Approximate width of right-aligned text
    ImGui::SetCursorPosX(width - textWidth);
    
    ImGui::Text("MiniGPU Engine | OpenGL 3.3");
    
    ImGui::End();
}