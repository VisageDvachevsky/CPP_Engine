#include "Editor.h"
#include "EditorCamera.h"
#include "SelectionManager.h"
#include "TransformGizmo.h"
#include "core/Window.h"
#include "core/Logger.h"
#include "core/Input.h"  // Add Input class include
#include "renderer/Renderer.h"
#include "scene/Scene.h"
#include "scene/Camera.h"
#include "gui/GUI.h"
#include "utils/Random.h"

#include <GLFW/glfw3.h>  // Add GLFW definitions for key codes

Editor::Editor(Window& window, Renderer& renderer, Scene& scene, Camera& camera)
    : m_window(window), m_renderer(renderer), m_scene(scene), m_camera(camera) {
    initializeEditor();
}

Editor::~Editor() = default;

void Editor::initializeEditor() {
    m_editorCamera = std::make_unique<EditorCamera>(m_camera);
    m_selectionManager = std::make_unique<SelectionManager>(m_scene);
    m_transformGizmo = std::make_unique<TransformGizmo>();
    
    // Set focus callback for selection manager
    m_selectionManager->setObjectFocusCallback(
        [this](const Vec3& position, float radius) {
            m_editorCamera->focusOnObject(position, radius);
        }
    );
    
    // Set gizmo activation callback
    m_selectionManager->setGizmoActivateCallback(
        [this]() {
            m_gizmoActive = true;
            LOG_INFO("Transform gizmo activated");
        }
    );
    
    m_gui = std::make_unique<GUI>(m_window, *this);
    
    LOG_INFO("Editor initialized with callbacks");
}

void Editor::update(float dt) {
    m_isViewportFocused = m_gui->isViewportFocused();
    m_isViewportHovered = m_gui->isViewportHovered();
    
    updateEditor(dt);
    
    // Process editor shortcuts
    processShortcuts();
}

void Editor::render() {
    renderEditor();
}

void Editor::updateEditor(float dt) {
    m_editorCamera->update(dt, m_isViewportFocused);
    m_selectionManager->update();
    
    if (m_selectionManager->hasSelection()) {
        Object* selectedObject = m_selectionManager->getSelectedObject();
        if (selectedObject) {
            m_transformGizmo->update(*selectedObject, m_camera);
        }
    }
    
    m_gui->update(m_scene, m_camera, m_renderer);
}

void Editor::renderEditor() {
    m_renderer.render(m_scene, m_camera);
    
    if (m_selectionManager->hasSelection() && (m_gizmoActive || m_mode == EditorMode::Edit)) {
        Object* selectedObject = m_selectionManager->getSelectedObject();
        if (selectedObject) {
            m_transformGizmo->render(m_renderer, m_camera);
        }
    }
    
    m_selectionManager->renderSelection(m_renderer, m_camera);
    
    m_gui->render();
}

void Editor::onWindowResize(int width, int height) {
    m_gui->onWindowResize(width, height);
}

void Editor::processShortcuts() {
    if (!m_isViewportFocused) {
        return;
    }
    
    // Get key states from Input system
    bool ctrlPressed = Input::isKeyHeld(GLFW_KEY_LEFT_CONTROL) || Input::isKeyHeld(GLFW_KEY_RIGHT_CONTROL);
    bool shiftPressed = Input::isKeyHeld(GLFW_KEY_LEFT_SHIFT) || Input::isKeyHeld(GLFW_KEY_RIGHT_SHIFT);
    bool altPressed = Input::isKeyHeld(GLFW_KEY_LEFT_ALT) || Input::isKeyHeld(GLFW_KEY_RIGHT_ALT);
    
    // Handle key presses
    
    // Transform tool shortcuts
    if (Input::isKeyPressed(GLFW_KEY_W)) {
        m_transformGizmo->setMode(GizmoMode::Translate);
        LOG_DEBUG("Switched to Translate gizmo mode");
    }
    else if (Input::isKeyPressed(GLFW_KEY_E)) {
        m_transformGizmo->setMode(GizmoMode::Rotate);
        LOG_DEBUG("Switched to Rotate gizmo mode");
    }
    else if (Input::isKeyPressed(GLFW_KEY_R)) {
        m_transformGizmo->setMode(GizmoMode::Scale);
        LOG_DEBUG("Switched to Scale gizmo mode");
    }
    
    // Coordinate space toggle
    if (Input::isKeyPressed(GLFW_KEY_T)) {
        GizmoSpace space = m_transformGizmo->getSpace();
        m_transformGizmo->setSpace(space == GizmoSpace::World ? GizmoSpace::Local : GizmoSpace::World);
        LOG_DEBUG("Toggled gizmo space to {}", 
                 m_transformGizmo->getSpace() == GizmoSpace::World ? "World" : "Local");
    }
    
    // Delete selected object
    if (Input::isKeyPressed(GLFW_KEY_DELETE)) {
        deleteSelectedObject();
    }
    
    // Duplicate selected object (Ctrl+D)
    if (ctrlPressed && Input::isKeyPressed(GLFW_KEY_D)) {
        duplicateSelectedObject();
    }
    
    // Focus on selected object (F key)
    if (Input::isKeyPressed(GLFW_KEY_F) && !ctrlPressed) {
        focusOnSelectedObject();
    }
    
    // Editor mode shortcuts
    if (Input::isKeyPressed(GLFW_KEY_1)) {
        setMode(EditorMode::Object);
    }
    else if (Input::isKeyPressed(GLFW_KEY_2)) {
        setMode(EditorMode::Edit);
    }
    else if (Input::isKeyPressed(GLFW_KEY_3)) {
        setMode(EditorMode::Play);
    }
}

void Editor::setMode(EditorMode mode) {
    if (m_mode == mode) return;
    
    m_mode = mode;
    
    // Automatically activate gizmo in Edit mode
    if (mode == EditorMode::Edit) {
        m_gizmoActive = true;
    }
    else if (mode == EditorMode::Play) {
        // TODO: Start game simulation
    }
    
    LOG_INFO("Editor mode set to {}", 
             mode == EditorMode::Object ? "Object" : 
             (mode == EditorMode::Edit ? "Edit" : "Play"));
}

void Editor::activateGizmo() {
    m_gizmoActive = true;
}

void Editor::deactivateGizmo() {
    m_gizmoActive = false;
}

void Editor::createPrimitive(ObjectType type) {
    std::string name;
    std::unique_ptr<Object> object;
    
    // Generate unique name
    int objectCount = m_scene.getObjectCount();
    
    switch (type) {
        case ObjectType::Sphere:
            name = "Sphere_" + std::to_string(objectCount);
            object = std::make_unique<Object>(name, type);
            
            // Set default properties for sphere
            object->getTransform().position = {0, 1, 0};
            object->getTransform().scale = {1, 1, 1};
            object->getMaterial().color = generateRandomColor();
            object->getMaterial().type = MaterialType::Diffuse;
            break;
            
        case ObjectType::Cube:
            name = "Cube_" + std::to_string(objectCount);
            object = std::make_unique<Object>(name, type);
            
            // Set default properties for cube - make it slightly bigger for visibility
            object->getTransform().position = {0, 0.5f, 0};
            object->getTransform().scale = {1.5f, 1.0f, 1.5f};
            object->getMaterial().color = generateRandomColor();
            object->getMaterial().type = MaterialType::Diffuse;
            break;
            
        case ObjectType::Plane:
            name = "Plane_" + std::to_string(objectCount);
            object = std::make_unique<Object>(name, type);
            
            // Set default properties for plane
            object->getTransform().position = {0, 0, 0};
            object->getTransform().scale = {10, 0.01f, 10}; // Very thin in Y dimension
            object->getMaterial().color = Vec3{0.8f, 0.8f, 0.8f};
            object->getMaterial().type = MaterialType::Diffuse;
            break;
            
        default:
            LOG_ERROR("Unknown primitive type");
            return;
    }
    
    // Get camera position and direction
    Vec3 cameraPos = m_camera.getPosition();
    Vec3 cameraDir = m_camera.getDirection();
    Vec3 cameraForward = Vec3{cameraDir.x, 0, cameraDir.z}; // Project onto XZ plane
    
    // Normalize if necessary
    if (cameraForward.lengthSq() > 0.0001f) {
        cameraForward.normalize();
    } else {
        cameraForward = Vec3{0, 0, 1};
    }
    
    // Place the object in front of the camera
    // Position 5 units in front of the camera, keeping Y coordinate as set above
    float distance = 5.0f;
    Vec3 placementPos = Vec3{
        cameraPos.x + cameraForward.x * distance,
        object->getTransform().position.y, // Keep the Y value we set earlier
        cameraPos.z + cameraForward.z * distance
    };
    
    // Keep planes at Y=0, but place other objects at the calculated position
    if (type != ObjectType::Plane) {
        object->getTransform().position = placementPos;
    }
    
    // Log position information for debugging
    LOG_INFO("Creating {} at position [{:.2f}, {:.2f}, {:.2f}]", 
             name, 
             object->getTransform().position.x,
             object->getTransform().position.y,
             object->getTransform().position.z);
    
    // Add to scene and select it
    Object* rawPtr = object.get();
    m_scene.addObject(std::move(object));
    m_selectionManager->selectObject(rawPtr);
    
    // Activate transform gizmo immediately to make it more obvious
    m_gizmoActive = true;
    
    // Set the editor to Edit mode to ensure gizmo is visible
    setMode(EditorMode::Edit);
    
    LOG_INFO("Created {} primitive '{}' and activated transform gizmo", 
             type == ObjectType::Sphere ? "Sphere" : 
             (type == ObjectType::Cube ? "Cube" : "Plane"), 
             name);
}

void Editor::deleteSelectedObject() {
    if (!m_selectionManager->hasSelection()) {
        return;
    }
    
    Object* selectedObject = m_selectionManager->getSelectedObject();
    if (!selectedObject) {
        return;
    }
    
    std::string objectName = selectedObject->getName();
    int index = m_scene.getObjectIndex(selectedObject);
    
    if (index >= 0) {
        m_selectionManager->deselectAll();
        m_scene.removeObject(index);
        LOG_INFO("Deleted object '{}'", objectName);
    }
}

void Editor::duplicateSelectedObject() {
    if (!m_selectionManager->hasSelection()) {
        return;
    }
    
    Object* selectedObject = m_selectionManager->getSelectedObject();
    if (!selectedObject) {
        return;
    }
    
    // Create a new object with the same properties
    std::string newName = selectedObject->getName() + "_Copy";
    std::unique_ptr<Object> newObject = std::make_unique<Object>(newName, selectedObject->getType());
    
    // Copy transform
    newObject->getTransform() = selectedObject->getTransform();
    
    // Offset position slightly to avoid overlapping
    newObject->getTransform().position.x += 2.0f;
    
    // Copy material
    newObject->getMaterial() = selectedObject->getMaterial();
    
    // Add to scene and select it
    Object* rawPtr = newObject.get();
    m_scene.addObject(std::move(newObject));
    m_selectionManager->selectObject(rawPtr);
    
    LOG_INFO("Duplicated object '{}' to '{}'", selectedObject->getName(), newName);
}

void Editor::focusOnSelectedObject() {
    if (!m_selectionManager->hasSelection()) {
        LOG_WARN("Cannot focus camera - no object selected");
        return;
    }
    
    Object* selectedObject = m_selectionManager->getSelectedObject();
    if (!selectedObject) {
        LOG_WARN("Selected object is null");
        return;
    }
    
    const Vec3& position = selectedObject->getTransform().position;
    // Calculate radius based on the largest dimension of the object
    float radius = std::max(
        std::max(selectedObject->getTransform().scale.x, 
                 selectedObject->getTransform().scale.y),
                 selectedObject->getTransform().scale.z);
    
    // Make sure radius isn't too small
    radius = std::max(radius, 0.5f);
    
    m_editorCamera->focusOnObject(position, radius);
    LOG_INFO("Focused camera on object '{}' at [{:.2f}, {:.2f}, {:.2f}], radius={:.2f}", 
             selectedObject->getName(),
             position.x, position.y, position.z,
             radius);
    
    // Automatically enable transform gizmo when focusing
    m_gizmoActive = true;
}

Vec3 Editor::generateRandomColor() {
    // Generate a pleasing random color
    float h = Random::uniform(0.0f, 360.0f);
    float s = Random::uniform(0.5f, 0.9f);
    float v = Random::uniform(0.7f, 1.0f);
    
    // Convert HSV to RGB
    float c = v * s;
    float x = c * (1 - std::abs(std::fmod(h / 60.0f, 2) - 1));
    float m = v - c;
    
    float r, g, b;
    if (h < 60) {
        r = c; g = x; b = 0;
    } else if (h < 120) {
        r = x; g = c; b = 0;
    } else if (h < 180) {
        r = 0; g = c; b = x;
    } else if (h < 240) {
        r = 0; g = x; b = c;
    } else if (h < 300) {
        r = x; g = 0; b = c;
    } else {
        r = c; g = 0; b = x;
    }
    
    return Vec3{r + m, g + m, b + m};
}

Renderer& Editor::getRenderer() {
    return m_renderer;
}