#include "Inspector.h"
#include "scene/Scene.h"
#include "scene/Object.h"
#include "scene/Material.h"

#include <imgui.h>
#include <string>

void Inspector::show(Scene& scene) {
    ImGui::Begin("Inspector");
    
    showSceneHierarchy(scene);
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    showObjectProperties(scene);
    
    ImGui::End();
}

void Inspector::showSceneHierarchy(Scene& scene) {
    ImGui::Text("Scene Hierarchy");
    ImGui::Separator();
    
    auto& objects = scene.getObjects();
    Object* selectedObject = scene.getSelectedObject();
    
    for (int i = 0; i < static_cast<int>(objects.size()); ++i) {
        auto& obj = objects[i];
        
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        if (obj.get() == selectedObject) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }
        
        ImGui::TreeNodeEx(obj->getName().c_str(), flags);
        
        if (ImGui::IsItemClicked()) {
            scene.setSelectedObject(obj.get());
        }
        
        // Material-specific properties
        if (material.type == MaterialType::Metal) {
            ImGui::SliderFloat("Roughness", &material.roughness, 0.0f, 1.0f);
        }
        else if (material.type == MaterialType::Dielectric) {
            ImGui::SliderFloat("IOR", &material.ior, 1.0f, 3.0f);
        }
        
        if (material.type == MaterialType::Diffuse || material.type == MaterialType::Metal) {
            ImGui::SliderFloat("Metalness", &material.metalness, 0.0f, 1.0f);
        }
    }
} Context menu
        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Delete")) {
                scene.removeObject(i);
            }
            ImGui::EndPopup();
        }
    }
    
    // Add object buttons
    ImGui::Spacing();
    if (ImGui::Button("Add Sphere")) {
        auto sphere = std::make_unique<Object>("New Sphere", ObjectType::Sphere);
        sphere->getTransform().position = {0, 1, 0};
        scene.addObject(std::move(sphere));
    }
    ImGui::SameLine();
    if (ImGui::Button("Add Plane")) {
        auto plane = std::make_unique<Object>("New Plane", ObjectType::Plane);
        scene.addObject(std::move(plane));
    }
}

void Inspector::showObjectProperties(Scene& scene) {
    Object* selectedObject = scene.getSelectedObject();
    if (!selectedObject) {
        ImGui::Text("No object selected");
        return;
    }
    
    ImGui::Text("Object Properties");
    ImGui::Separator();
    
    // Object name
    char name[256];
    strcpy_s(name, selectedObject->getName().c_str());
    if (ImGui::InputText("Name", name, sizeof(name))) {
        selectedObject->setName(std::string(name));
    }
    
    // Transform
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        Transform& transform = selectedObject->getTransform();
        
        ImGui::DragFloat3("Position", transform.position.data(), 0.1f);
        ImGui::DragFloat3("Rotation", transform.rotation.data(), 1.0f);
        ImGui::DragFloat3("Scale", transform.scale.data(), 0.1f, 0.1f, 10.0f);
    }
    
    // Material
    if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
        Material& material = selectedObject->getMaterial();
        
        ImGui::ColorEdit3("Color", material.color.data());
        
        // Material type
        const char* materialTypes[] = {"Diffuse", "Metal", "Dielectric"};
        int currentType = static_cast<int>(material.type);
        if (ImGui::Combo("Type", &currentType, materialTypes, 3)) {
            material.type = static_cast<MaterialType>(currentType);
        }
        
        //