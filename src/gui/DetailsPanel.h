#pragma once

class Scene;

class DetailsPanel {
public:
    DetailsPanel() = default;
    ~DetailsPanel() = default;
    
    void show(Scene& scene);

private:
    void showObjectDetails(Scene& scene);
    void showTransformSection(class Object& object);
    void showMaterialSection(class Object& object);
};