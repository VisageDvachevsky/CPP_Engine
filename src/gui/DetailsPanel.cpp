#include "DetailsPanel.h"
#include "scene/Scene.h"
#include "scene/Object.h"
#include "scene/Material.h"

#include <imgui.h>

void DetailsPanel::show(Scene& scene) {
    ImGui::Begin("Details");
    
    Object* selectedObject = scene.getSelectedObject();
    if (!selectedObject) {
        ImGui::Text("No object selected");
        ImGui::Text("Select an object in the viewport or outliner to see its properties.");
        ImGui::End();
        return;
    }
    
    showObjectDetails(scene);
    
    ImGui::End();
}

void DetailsPanel::showObjectDetails(Scene& scene) {
    Object* object = scene.getSelectedObject();
    if (!object) return;
    
    // Object header
    ImGui::Text("Object: %s", object->getName().c_str());
    ImGui::Separator();
    
    // Object name
    char name[256];
    strcpy_s(name, sizeof(name), object->getName().c_str());
    if (ImGui::InputText("Name", name, sizeof(name))) {
        object->setName(std::string(name));
    }
    
    // Object type (read-only)
    const char* typeNames[] = {"Sphere", "Plane", "Cube"};
    int typeIndex = static_cast<int>(object->getType());
    ImGui::Text("Type: %s", typeNames[typeIndex]);
    
    ImGui::Spacing();
    
    // Transform section
    showTransformSection(*object);
    
    ImGui::Spacing();
    
    // Material section  
    showMaterialSection(*object);
}

void DetailsPanel::showTransformSection(Object& object) {
    if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
        Transform& transform = object.getTransform();
        
        ImGui::Text("Position");
        ImGui::DragFloat3("##Position", transform.position.data(), 0.1f);
        
        ImGui::Text("Rotation");
        ImGui::DragFloat3("##Rotation", transform.rotation.data(), 1.0f, -180.0f, 180.0f);
        
        ImGui::Text("Scale");
        ImGui::DragFloat3("##Scale", transform.scale.data(), 0.1f, 0.001f, 100.0f);
        
        // Quick buttons
        if (ImGui::Button("Reset Position")) {
            transform.position = Vec3{0, 0, 0};
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset Rotation")) {
            transform.rotation = Vec3{0, 0, 0};
        }
        ImGui::SameLine();
        if (ImGui::Button("Reset Scale")) {
            transform.scale = Vec3{1, 1, 1};
        }
    }
}

void DetailsPanel::showMaterialSection(Object& object) {
    if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
        Material& material = object.getMaterial();
        
        // Color picker
        ImGui::ColorEdit3("Color", material.color.data());
        
        // Material type
        const char* materialTypes[] = {"Diffuse", "Metal", "Dielectric"};
        int currentType = static_cast<int>(material.type);
        if (ImGui::Combo("Material Type", &currentType, materialTypes, 3)) {
            material.type = static_cast<MaterialType>(currentType);
        }
        
        // Material-specific properties
        switch (material.type) {
            case MaterialType::Metal:
                ImGui::SliderFloat("Roughness", &material.roughness, 0.0f, 1.0f);
                ImGui::SliderFloat("Metalness", &material.metalness, 0.0f, 1.0f);
                break;
                
            case MaterialType::Dielectric:
                ImGui::SliderFloat("IOR", &material.ior, 1.0f, 3.0f);
                ImGui::Text("Common IOR values:");
                ImGui::BulletText("Air: 1.0");
                ImGui::BulletText("Water: 1.33");
                ImGui::BulletText("Glass: 1.5");
                ImGui::BulletText("Diamond: 2.42");
                break;
                
            case MaterialType::Diffuse:
            default:
                // No additional properties for diffuse
                break;
        }
        
        // Material presets
        ImGui::Separator();
        ImGui::Text("Material Presets:");
        
        if (ImGui::Button("Gold")) {
            material.type = MaterialType::Metal;
            material.color = Vec3{1.0f, 0.8f, 0.0f};
            material.roughness = 0.1f;
            material.metalness = 1.0f;
        }
        ImGui::SameLine();
        if (ImGui::Button("Silver")) {
            material.type = MaterialType::Metal;
            material.color = Vec3{0.9f, 0.9f, 0.9f};
            material.roughness = 0.05f;
            material.metalness = 1.0f;
        }
        ImGui::SameLine();
        if (ImGui::Button("Glass")) {
            material.type = MaterialType::Dielectric;
            material.color = Vec3{1.0f, 1.0f, 1.0f};
            material.ior = 1.5f;
        }
        
        if (ImGui::Button("Rubber")) {
            material.type = MaterialType::Diffuse;
            material.color = Vec3{0.2f, 0.2f, 0.2f};
        }
        ImGui::SameLine();
        if (ImGui::Button("Plastic")) {
            material.type = MaterialType::Diffuse;
            material.color = Vec3{0.8f, 0.2f, 0.2f};
        }
    }
}