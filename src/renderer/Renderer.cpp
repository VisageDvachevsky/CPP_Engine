#include "Renderer.h"
#include "scene/Scene.h"
#include "scene/Camera.h"
#include "scene/Object.h"
#include "core/Logger.h"
#include "core/Time.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>

Renderer::Renderer() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    createQuad();
    createGrid();
    
    // Load shaders
    m_pathTracerShader = std::make_unique<Shader>();
    if (!m_pathTracerShader->loadFromFiles("shaders/pathtracer.vert", "shaders/pathtracer.frag")) {
        LOG_ERROR("Failed to load path tracer shader");
    }
    
    m_wireframeShader = std::make_unique<Shader>();
    if (!m_wireframeShader->loadFromFiles("shaders/wireframe.vert", "shaders/wireframe.frag")) {
        LOG_WARN("Failed to load wireframe shader - selection outlines disabled");
    }
    
    m_gridShader = std::make_unique<Shader>();
    if (!m_gridShader->loadFromFiles("shaders/grid.vert", "shaders/grid.frag")) {
        LOG_WARN("Failed to load grid shader - grid disabled");
    }
    
    LOG_INFO("Renderer initialized");
}

Renderer::~Renderer() {
    if (m_quadVAO) {
        glDeleteVertexArrays(1, &m_quadVAO);
        glDeleteBuffers(1, &m_quadVBO);
    }
    if (m_gridVAO) {
        glDeleteVertexArrays(1, &m_gridVAO);
        glDeleteBuffers(1, &m_gridVBO);
        glDeleteBuffers(1, &m_gridIBO);
    }
}

void Renderer::createQuad() {
    // Fullscreen quad vertices
    float vertices[] = {
        // positions   // texCoords
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f,  0.0f, 1.0f
    };
    
    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);
    
    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Texture coords
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
}

void Renderer::createGrid() {
    const int gridSize = 100;
    const float gridSpacing = 1.0f;
    
    std::vector<float> gridVertices;
    std::vector<unsigned int> gridIndices;
    
    // Generate grid lines
    for (int i = -gridSize; i <= gridSize; ++i) {
        float pos = i * gridSpacing;
        
        // Vertical lines
        gridVertices.insert(gridVertices.end(), {pos, 0.0f, -gridSize * gridSpacing});
        gridVertices.insert(gridVertices.end(), {pos, 0.0f, gridSize * gridSpacing});
        
        // Horizontal lines
        gridVertices.insert(gridVertices.end(), {-gridSize * gridSpacing, 0.0f, pos});
        gridVertices.insert(gridVertices.end(), {gridSize * gridSpacing, 0.0f, pos});
    }
    
    // Generate indices
    for (unsigned int i = 0; i < gridVertices.size() / 3; ++i) {
        gridIndices.push_back(i);
    }
    
    glGenVertexArrays(1, &m_gridVAO);
    glGenBuffers(1, &m_gridVBO);
    glGenBuffers(1, &m_gridIBO);
    
    glBindVertexArray(m_gridVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_gridVBO);
    glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_gridIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, gridIndices.size() * sizeof(unsigned int), gridIndices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
}

void Renderer::clear() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_drawCalls = 0;
}

void Renderer::render(const Scene& scene, const Camera& camera) {
    if (!m_pathTracerShader->isValid()) {
        return;
    }
    
    // Render grid first
    renderGrid(camera);
    
    // Get viewport size
    Vec2 viewportSize = m_viewportSize;
    
    m_pathTracerShader->use();
    
    // Camera uniforms
    Vec3 pos = camera.getPosition();
    Vec3 dir = camera.getDirection();
    Vec3 up = camera.getUp();
    Vec3 right = camera.getRight();
    
    m_pathTracerShader->setVec3("u_cameraPos", pos);
    m_pathTracerShader->setVec3("u_cameraDir", dir);
    m_pathTracerShader->setVec3("u_cameraUp", up);
    m_pathTracerShader->setVec3("u_cameraRight", right);
    m_pathTracerShader->setFloat("u_fov", camera.getFov());
    
    // Resolution and time
    m_pathTracerShader->setVec3("u_resolution", Vec3(viewportSize.x, viewportSize.y, 0));
    m_pathTracerShader->setFloat("u_time", Time::getTime());
    
    // Render settings
    m_pathTracerShader->setInt("u_maxBounces", m_maxBounces);
    m_pathTracerShader->setInt("u_samplesPerPixel", m_samplesPerPixel);
    
    updateUniforms(scene, camera);
    
    // Render fullscreen quad
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    
    m_pathTracerShader->unuse();
    m_drawCalls++;
    
    updateStats();
}

void Renderer::renderGrid(const Camera& camera) {
    if (!m_gridShader->isValid() || !m_gridVAO) return;
    
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);
    
    m_gridShader->use();
    
    Mat4 view = camera.getViewMatrix();
    Mat4 proj = camera.getProjectionMatrix(m_viewportSize.x / m_viewportSize.y);
    Mat4 mvp = proj * view;
    
    m_gridShader->setMat4("u_mvp", mvp);
    m_gridShader->setVec3("u_cameraPos", camera.getPosition());
    m_gridShader->setVec3("u_gridColor", Vec3{0.3f, 0.3f, 0.3f});
    m_gridShader->setFloat("u_fadeDistance", 50.0f);
    
    glBindVertexArray(m_gridVAO);
    glDrawElements(GL_LINES, (100 * 2 + 1) * 4, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    
    m_gridShader->unuse();
    
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    
    m_drawCalls++;
}

void Renderer::renderSelectionOutline(const Object& object, const Camera& camera) {
    if (!m_wireframeShader->isValid()) return;
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(2.0f);
    
    m_wireframeShader->use();
    
    Mat4 model = Mat4::translate(object.getTransform().position) * 
                 Mat4::scale(object.getTransform().scale);
    Mat4 view = camera.getViewMatrix();
    Mat4 proj = camera.getProjectionMatrix(m_viewportSize.x / m_viewportSize.y);
    Mat4 mvp = proj * view * model;
    
    m_wireframeShader->setMat4("u_mvp", mvp);
    m_wireframeShader->setVec3("u_color", Vec3{1.0f, 0.5f, 0.0f}); // Orange selection
    
    renderObjectWireframe(object);
    
    m_wireframeShader->unuse();
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glLineWidth(1.0f);
}

void Renderer::renderHoverOutline(const Object& object, const Camera& camera) {
    if (!m_wireframeShader->isValid()) return;
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(1.5f);
    
    m_wireframeShader->use();
    
    Mat4 model = Mat4::translate(object.getTransform().position) * 
                 Mat4::scale(object.getTransform().scale);
    Mat4 view = camera.getViewMatrix();
    Mat4 proj = camera.getProjectionMatrix(m_viewportSize.x / m_viewportSize.y);
    Mat4 mvp = proj * view * model;
    
    m_wireframeShader->setMat4("u_mvp", mvp);
    m_wireframeShader->setVec3("u_color", Vec3{0.8f, 0.8f, 0.8f}); // Gray hover
    
    renderObjectWireframe(object);
    
    m_wireframeShader->unuse();
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glLineWidth(1.0f);
}

void Renderer::renderObjectWireframe(const Object& object) {
    // TODO: Implement proper wireframe rendering for different object types
    // This would need actual mesh generation for spheres, cubes, etc.
    switch (object.getType()) {
        case ObjectType::Sphere:
            // Generate and render sphere wireframe mesh
            break;
        case ObjectType::Cube:
            // Generate and render cube wireframe mesh
            break;
        case ObjectType::Plane:
            // Generate and render plane wireframe mesh
            break;
    }
}

void Renderer::updateUniforms(const Scene& scene, const Camera& camera) {
    const auto& objects = scene.getObjects();
    
    int sphereCount = 0;
    int planeCount = 0;
    
    for (const auto& obj : objects) {
        if (obj->getType() == ObjectType::Sphere && sphereCount < 16) {
            std::string base = "u_spheres[" + std::to_string(sphereCount) + "]";
            
            Vec3 pos = obj->getTransform().position;
            Vec3 color = obj->getMaterial().color;
            float radius = obj->getTransform().scale.x; // Use consistent API
            
            m_pathTracerShader->setVec3(base + ".center", pos);
            m_pathTracerShader->setFloat(base + ".radius", radius);
            m_pathTracerShader->setVec3(base + ".color", color);
            m_pathTracerShader->setInt(base + ".materialType", static_cast<int>(obj->getMaterial().type));
            m_pathTracerShader->setFloat(base + ".roughness", obj->getMaterial().roughness);
            m_pathTracerShader->setFloat(base + ".ior", obj->getMaterial().ior);
            
            sphereCount++;
        }
        else if (obj->getType() == ObjectType::Plane && planeCount < 4) {
            std::string base = "u_planes[" + std::to_string(planeCount) + "]";
            
            Vec3 pos = obj->getTransform().position;
            Vec3 normal = Vec3{0, 1, 0}; // Default up normal
            Vec3 color = obj->getMaterial().color;
            
            m_pathTracerShader->setVec3(base + ".point", pos);
            m_pathTracerShader->setVec3(base + ".normal", normal);
            m_pathTracerShader->setVec3(base + ".color", color);
            m_pathTracerShader->setInt(base + ".materialType", static_cast<int>(obj->getMaterial().type));
            m_pathTracerShader->setFloat(base + ".roughness", obj->getMaterial().roughness);
            
            planeCount++;
        }
    }
    
    m_pathTracerShader->setInt("u_numSpheres", sphereCount);
    m_pathTracerShader->setInt("u_numPlanes", planeCount);
}

void Renderer::updateStats() {
    m_fps = Time::getFPS(); // Use consistent time API
}

void Renderer::reloadShaders() {
    LOG_INFO("Reloading shaders...");
    
    if (m_pathTracerShader->loadFromFiles("shaders/pathtracer.vert", "shaders/pathtracer.frag")) {
        LOG_INFO("Path tracer shader reloaded successfully");
    } else {
        LOG_ERROR("Failed to reload path tracer shader");
    }
    
    if (m_wireframeShader->loadFromFiles("shaders/wireframe.vert", "shaders/wireframe.frag")) {
        LOG_INFO("Wireframe shader reloaded successfully");
    } else {
        LOG_WARN("Failed to reload wireframe shader");
    }
    
    if (m_gridShader->loadFromFiles("shaders/grid.vert", "shaders/grid.frag")) {
        LOG_INFO("Grid shader reloaded successfully");
    } else {
        LOG_WARN("Failed to reload grid shader");
    }
}