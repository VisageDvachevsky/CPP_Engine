#pragma once

class Scene;

class Inspector {
public:
    Inspector() = default;
    ~Inspector() = default;
    
    void show(Scene& scene);

private:
    void showSceneHierarchy(Scene& scene);
    void showObjectProperties(Scene& scene);
    void showRenderSettings();
};