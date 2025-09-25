#include "ContentBrowser.h"
#include <imgui.h>
#include <filesystem>

ContentBrowser::ContentBrowser() : m_currentPath("assets") {
}

void ContentBrowser::show() {
    ImGui::Begin("Content Browser");
    
    showPathBar();
    ImGui::Separator();
    
    if (ImGui::BeginTable("ContentBrowserTable", 2, ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn("Directories", ImGuiTableColumnFlags_WidthFixed, 200.0f);
        ImGui::TableSetupColumn("Files", ImGuiTableColumnFlags_WidthStretch);
        
        ImGui::TableNextRow();
        
        // Directory tree
        ImGui::TableNextColumn();
        showDirectoryTree();
        
        // File grid
        ImGui::TableNextColumn();
        showFileGrid();
        
        ImGui::EndTable();
    }
    
    ImGui::End();
}

void ContentBrowser::showPathBar() {
    if (ImGui::Button("< Back")) {
        // TODO: Navigate back
    }
    ImGui::SameLine();
    if (ImGui::Button("^ Up")) {
        std::filesystem::path path(m_currentPath);
        if (path.has_parent_path()) {
            m_currentPath = path.parent_path().string();
        }
    }
    ImGui::SameLine();
    
    // Current path
    ImGui::Text("Path: %s", m_currentPath.c_str());
}

void ContentBrowser::showDirectoryTree() {
    ImGui::Text("Folders");
    ImGui::Separator();
    
    // Root folders
    if (ImGui::TreeNodeEx("Assets", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Selectable("Textures")) {
            m_currentPath = "assets/textures";
        }
        if (ImGui::Selectable("Materials")) {
            m_currentPath = "assets/materials";
        }
        if (ImGui::Selectable("Models")) {
            m_currentPath = "assets/models";
        }
        if (ImGui::Selectable("Shaders")) {
            m_currentPath = "shaders";
        }
        ImGui::TreePop();
    }
}

void ContentBrowser::showFileGrid() {
    ImGui::Text("Files in: %s", m_currentPath.c_str());
    ImGui::Separator();
    
    try {
        if (std::filesystem::exists(m_currentPath)) {
            for (const auto& entry : std::filesystem::directory_iterator(m_currentPath)) {
                if (entry.is_regular_file()) {
                    std::string filename = entry.path().filename().string();
                    
                    std::string ext = entry.path().extension().string();
                    const char* icon = "📄";
                    if (ext == ".frag" || ext == ".vert" || ext == ".glsl") icon = "🔧";
                    else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") icon = "🖼️";
                    else if (ext == ".obj" || ext == ".fbx" || ext == ".gltf") icon = "🎲";
                    
                    std::string label = std::string(icon) + " " + filename;
                    
                    if (ImGui::Selectable(label.c_str())) {
                        // TODO: Handle file selection
                    }
                    
                    // Context menu
                    if (ImGui::BeginPopupContextItem()) {
                        ImGui::Text("File: %s", filename.c_str());
                        ImGui::Separator();
                        if (ImGui::MenuItem("Open")) {
                            // TODO: Open file
                        }
                        if (ImGui::MenuItem("Rename")) {
                            // TODO: Rename file
                        }
                        if (ImGui::MenuItem("Delete")) {
                            // TODO: Delete file
                        }
                        ImGui::EndPopup();
                    }
                }
            }
        } else {
            ImGui::Text("Directory does not exist");
        }
    } catch (const std::exception& e) {
        ImGui::Text("Error reading directory: %s", e.what());
    }
}