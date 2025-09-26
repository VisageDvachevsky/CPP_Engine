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
    
    // Устанавливаем колбэк для фокусировки на объекте
    m_selectionManager->setObjectFocusCallback(
        [this](const Vec3& position, float radius) {
            m_editorCamera->focusOnObject(position, radius);
        }
    );
    
    m_gui = std::make_unique<GUI>(m_window, *this);
    
    LOG_INFO("Editor systems initialized with focus callback");
}

void Editor::update(float dt) {
    m_isViewportFocused = m_gui->isViewportFocused();
    m_isViewportHovered = m_gui->isViewportHovered();
    
    LOG_DEBUG("Editor update: viewport focused={}, hovered={}", 
              m_isViewportFocused, m_isViewportHovered);
              
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
        Object* selectedObject = m_selectionManager->getSelectedObject();
        if (selectedObject) {
            LOG_INFO("Editor: Rendering transform gizmo for object '{}'", selectedObject->getName());
            m_transformGizmo->update(*selectedObject, m_camera);
            m_transformGizmo->render(m_renderer, m_camera);
        }
    }
    
    m_selectionManager->renderSelection(m_renderer, m_camera);
    
    m_selectionManager->renderSelection(m_renderer, m_camera);
    
    m_gui->render();
}

void Editor::onWindowResize(int width, int height) {
    m_gui->onWindowResize(width, height);
}