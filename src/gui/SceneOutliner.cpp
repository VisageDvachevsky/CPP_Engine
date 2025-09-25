#include "SceneOutliner.h"
#include "scene/Scene.h"
#include "scene/Object.h"
#include "core/Logger.h"

#include <imgui.h>
#include <string>

void SceneOutliner::show(Scene& scene) {
    ImGui::Begin("Scene Outliner");
    
    // Header with create button
    if (ImGui::Button("Create")) {
        m_showCreateMenu = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Refresh")) {
        // Refresh scene hierarchy
    }
    
    ImGui::Separator();
    
    // Show object hierarchy
    showObjectHierarchy(scene);
    
    // Handle object creation menu
    if (m_showCreateMenu) {
        handleObjectCreation(scene);
    }
    
    // Context menu
    showContextMenu(scene);
    
    ImGui::End();
}

void SceneOutliner::showObjectHierarchy(Scene& scene) {
    auto& objects = scene.getObjects();
    Object* selectedObject = scene.getSelectedObject();
    
    // Scene root
    if (ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_DefaultOpen)) {
        
        for (int i = 0; i < static_cast<int>(objects.size()); ++i) {
            auto& obj = objects[i];
            
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | 
                                     ImGuiTreeNodeFlags_NoTreePushOnOpen |
                                     ImGuiTreeNodeFlags_SpanFullWidth;
            
            if (obj.get() == selectedObject) {
                flags |= ImGuiTreeNodeFlags_Selected;
            }
            
            // Object icon based on type
            const char* icon = "🟡"; // Default
            switch (obj->getType()) {
                case ObjectType::Sphere: icon = "🔵"; break;
                case ObjectType::Plane: icon = "⬜"; break;
                case ObjectType::Cube: icon = "🟦"; break;
            }
            
            std::string label = std::string(icon) + " " + obj->getName();
            ImGui::TreeNodeEx(label.c_str(), flags);
            
            if (ImGui::IsItemClicked()) {
                scene.setSelectedObject(obj.get());
            }
            
            // Drag and drop (future feature)
            if (ImGui::BeginDragDropSource()) {
                ImGui::SetDragDropPayload("SCENE_OBJECT", &i, sizeof(int));
                ImGui::Text("Moving %s", obj->getName().c_str());
                ImGui::EndDragDropSource();
            }
            
            // Context menu for individual objects
            if (ImGui::BeginPopupContextItem()) {
                ImGui::Text("Object: %s", obj->getName().c_str());
                ImGui::Separator();
                
                if (ImGui::MenuItem("Rename")) {
                    // TODO: Implement rename dialog
                }
                
                if (ImGui::MenuItem("Duplicate")) {
                    // TODO: Implement object duplication
                }
                
                if (ImGui::MenuItem("Delete", "Del")) {
                    scene.removeObject(i);
                }
                
                ImGui::EndPopup();
            }
        }
        
        ImGui::TreePop();
    }
}

void SceneOutliner::showContextMenu(Scene& scene) {
    // Right-click on empty space
    if (ImGui::BeginPopupContextWindow("SceneOutlinerContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
        if (ImGui::BeginMenu("Create")) {
            if (ImGui::MenuItem("Sphere")) {
                auto sphere = std::make_unique<Object>("Sphere", ObjectType::Sphere);
                sphere->getTransform().position = {0, 1, 0};
                sphere->getMaterial().color = {0.7f, 0.3f, 0.3f};
                scene.addObject(std::move(sphere));
                LOG_INFO("Created new sphere");
            }
            
            if (ImGui::MenuItem("Cube")) {
                auto cube = std::make_unique<Object>("Cube", ObjectType::Cube);
                cube->getTransform().position = {0, 0.5f, 0};
                cube->getMaterial().color = {0.3f, 0.7f, 0.3f};
                scene.addObject(std::move(cube));
                LOG_INFO("Created new cube");
            }
            
            if (ImGui::MenuItem("Plane")) {
                auto plane = std::make_unique<Object>("Plane", ObjectType::Plane);
                plane->getTransform().position = {0, 0, 0};
                plane->getTransform().scale = {10, 1, 10};
                plane->getMaterial().color = {0.5f, 0.5f, 0.5f};
                scene.addObject(std::move(plane));
                LOG_INFO("Created new plane");
            }
            
            ImGui::EndMenu();
        }
        
        if (ImGui::MenuItem("Paste")) {
            // TODO: Implement paste functionality
        }
        
        ImGui::EndPopup();
    }
}

void SceneOutliner::handleObjectCreation(Scene& scene) {
    ImGui::OpenPopup("Create Object");
    
    if (ImGui::BeginPopup("Create Object")) {
        ImGui::Text("Create New Object");
        ImGui::Separator();
        
        if (ImGui::Selectable("Sphere")) {
            auto sphere = std::make_unique<Object>("New Sphere", ObjectType::Sphere);
            sphere->getTransform().position = {0, 1, 0};
            sphere->getMaterial().type = MaterialType::Diffuse;
            sphere->getMaterial().color = {0.7f, 0.3f, 0.3f};
            scene.addObject(std::move(sphere));
            m_showCreateMenu = false;
            LOG_INFO("Created sphere from menu");
        }
        
        if (ImGui::Selectable("Cube")) {
            auto cube = std::make_unique<Object>("New Cube", ObjectType::Cube);
            cube->getTransform().position = {0, 0.5f, 0};
            cube->getMaterial().type = MaterialType::Diffuse;
            cube->getMaterial().color = {0.3f, 0.7f, 0.3f};
            scene.addObject(std::move(cube));
            m_showCreateMenu = false;
            LOG_INFO("Created cube from menu");
        }
        
        if (ImGui::Selectable("Plane")) {
            auto plane = std::make_unique<Object>("New Plane", ObjectType::Plane);
            plane->getTransform().position = {0, 0, 0};
            plane->getTransform().scale = {5, 1, 5};
            plane->getMaterial().type = MaterialType::Diffuse;
            plane->getMaterial().color = {0.8f, 0.8f, 0.8f};
            scene.addObject(std::move(plane));
            m_showCreateMenu = false;
            LOG_INFO("Created plane from menu");
        }
        
        if (ImGui::Selectable("Cancel")) {
            m_showCreateMenu = false;
        }
        
        ImGui::EndPopup();
    } else {
        m_showCreateMenu = false;
    }
}