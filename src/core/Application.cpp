#include "Application.h"
#include "Window.h"
#include "Input.h"
#include "Logger.h"
#include "Time.h"
#include "renderer/Renderer.h"
#include "scene/Scene.h"
#include "scene/Camera.h"
#include "editor/Editor.h"
#include "utils/FileWatcher.h"

#include <GLFW/glfw3.h>
#include <stdexcept>

Application* Application::s_instance = nullptr;

Application::Application() {
    s_instance = this;
    init();
}

Application::~Application() {
    cleanup();
    s_instance = nullptr;
}

void Application::init() {
    LOG_INFO("Initializing MiniGPU Engine...");
    
    m_window = std::make_unique<Window>(1920, 1080, "MiniGPU Engine - Unreal Style Editor");

    Input::init(m_window->handle());


    m_renderer = std::make_unique<Renderer>();
    m_scene = std::make_unique<Scene>();
    m_camera = std::make_unique<Camera>();
    
    m_editor = std::make_unique<Editor>(*m_window, *m_renderer, *m_scene, *m_camera);
    
    m_fileWatcher = std::make_unique<FileWatcher>("shaders");
    m_fileWatcher->setCallback([this](const std::string& path) {
        LOG_INFO("Shader changed: {}", path);
        m_renderer->reloadShaders();
    });
    m_scene->createDefaultScene();
    m_camera->setPosition({5, 5, 5});
    m_camera->lookAt({0, 0, 0});
    
    // Setup resize callback
    m_window->setResizeCallback([this](int width, int height) {
        m_renderer->setViewportSize(width, height);
        m_editor->onWindowResize(width, height);
    });
    
    LOG_INFO("Engine initialized successfully");
}

void Application::run() {
    LOG_INFO("Starting main loop...");
    
    Time::init();
    
    while (!m_window->shouldClose() && m_running) {
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - m_lastFrameTime;
        m_lastFrameTime = currentTime;
        
        Time::update(deltaTime);
        
        handleEvents();
        update(deltaTime);
        render();
        
        m_window->swapBuffers();
    }
    
    LOG_INFO("Main loop ended");
}

void Application::handleEvents() {
    m_window->pollEvents();
    Input::update();
    m_fileWatcher->update();
}

void Application::update(float dt) {

    m_scene->update(dt);
    m_editor->update(dt);
    
    // Handle window close request
    if (Input::isKeyPressed(GLFW_KEY_ESCAPE)) {
        m_running = false;
    }
}

void Application::render() {
    m_renderer->clear();
    m_editor->render();
}

void Application::cleanup() {
    LOG_INFO("Cleaning up application...");
    
    m_fileWatcher.reset();
    m_editor.reset();
    m_camera.reset();
    m_scene.reset();
    m_renderer.reset();
    m_window.reset();
    
    LOG_INFO("Application cleanup complete");
}