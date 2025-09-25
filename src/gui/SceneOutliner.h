#pragma once

class Scene;

class SceneOutliner {
public:
    SceneOutliner() = default;
    ~SceneOutliner() = default;
    
    void show(Scene& scene);

private:
    void showObjectHierarchy(Scene& scene);
    void showContextMenu(Scene& scene);
    void handleObjectCreation(Scene& scene);
    
    bool m_showCreateMenu = false;
};