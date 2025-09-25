#include "Editor.h"
#include "EditorCamera.h"
#include "SelectionManager.h"
#include "TransformGizmo.h"
#include "core/Window.h"
#include "core/Logger.h"
#include "renderer/Renderer.h"
#include "scene/Scene.h"
#include "scene/Camera.h"
#include "gui/GUI.h"

Editor::Editor(Window& window, Renderer& renderer, Scene& scene, Camera& camera)
    : m_window(window), m_renderer(renderer), m_scene(scene), m_camera(camera) {
    initializeEditor();
}

Editor::~Editor() = default;

void Editor::initializeEditor() {
    m_editorCamera = std::make_unique<EditorCamera>(m_camera);
    m_selectionManager = std::make_unique<SelectionManager>(m_scene);
    m_transformGizmo = std::make_unique<TransformGizmo>();
    m_gui = std::make_unique<GUI>(m_window, *this);
    
    LOG_INFO("Editor systems initialized");
}

void Editor::update(float dt) {
    updateEditor(dt);
}

void Editor::render() {
    renderEditor();
}

void Editor::updateEditor(float dt) {
    m_editorCamera->update(dt, m_isViewportFocused);
    
    m_selectionManager->update();
    
    if (m_selectionManager->hasSelection()) {
        m_transformGizmo->update(*m_selectionManager->getSelectedObject(), m_camera);
    }
    
    m_gui->update(m_scene, m_camera, m_renderer);
}

void Editor::renderEditor() {
    m_renderer.render(m_scene, m_camera);
    
    if (m_selectionManager->hasSelection() && m_isViewportFocused) {
        m_transformGizmo->render(m_renderer, m_camera);
    }
    
    m_selectionManager->renderSelection(m_renderer, m_camera);
    
    m_gui->render();
}

void Editor::onWindowResize(int width, int height) {
    m_gui->onWindowResize(width, height);
}