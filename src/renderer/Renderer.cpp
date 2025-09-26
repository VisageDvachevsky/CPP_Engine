#include "Renderer.h"
#include "scene/Scene.h"
#include "scene/Camera.h"
#include "scene/Object.h"
#include "core/Logger.h"
#include "core/Time.h"
#include "utils/ResourceManager.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <sstream>

Renderer::Renderer() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    createQuad();
    createGrid();
    
    // Initialize primitive renderer first
    initializePrimitiveRenderer();
    
    m_pathTracerShader = ResourceManager::instance().loadShader(
        "pathtracer", "shaders/pathtracer.vert", "shaders/pathtracer.frag");
    
    m_wireframeShader = ResourceManager::instance().loadShader(
        "wireframe", "shaders/wireframe.vert", "shaders/wireframe.frag");
    
    m_gridShader = ResourceManager::instance().loadShader(
        "grid", "shaders/grid.vert", "shaders/grid.frag");
    
    // Log shader loading status
    if (m_pathTracerShader && m_pathTracerShader->isValid()) {
        LOG_INFO("PathTracer shader loaded successfully");
    } else {
        LOG_ERROR("Failed to load PathTracer shader - will render primitives only");
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
    float vertices[] = {
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, 0.0f, 1.0f
    };
    
    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);
    
    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
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
        
        gridVertices.insert(gridVertices.end(), {pos, 0.0f, -gridSize * gridSpacing});
        gridVertices.insert(gridVertices.end(), {pos, 0.0f, gridSize * gridSpacing});
        
        gridVertices.insert(gridVertices.end(), {-gridSize * gridSpacing, 0.0f, pos});
        gridVertices.insert(gridVertices.end(), {gridSize * gridSpacing, 0.0f, pos});
    }
    
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

void Renderer::initializePrimitiveRenderer() {
    m_primitiveRenderer = std::make_unique<PrimitiveRenderer>();
    m_primitiveRenderer->initialize();
    LOG_INFO("Primitive renderer initialized");
}

void Renderer::render(const Scene& scene, const Camera& camera) {
    m_drawCalls = 0;

    if (!m_pathTracerShader || !m_pathTracerShader->isValid()) {
        LOG_ERROR("PathTracer shader is invalid, skipping render");
        // Still render primitives if possible
        if (m_primitiveRenderer) {
            renderPrimitives(scene, camera);
        }
        if (m_gridShader && m_gridShader->isValid()) {
            renderGrid(camera);
        }
        updateStats();
        return;
    }
    
    Vec2 viewportSize = m_viewportSize;
    
    m_pathTracerShader->use();
    
    Vec3 pos = camera.getPosition();
    Vec3 dir = camera.getDirection();
    Vec3 up = camera.getUp();
    Vec3 right = camera.getRight();

    m_pathTracerShader->setVec3("u_cameraPos", pos);
    m_pathTracerShader->setVec3("u_cameraDir", dir);
    m_pathTracerShader->setVec3("u_cameraUp", up);
    m_pathTracerShader->setVec3("u_cameraRight", right);
    m_pathTracerShader->setFloat("u_fov", camera.getFov());
    
    m_pathTracerShader->setVec2("u_resolution", viewportSize); 
    m_pathTracerShader->setFloat("u_time", Time::getTime());
    
    m_pathTracerShader->setInt("u_maxBounces", m_maxBounces);
    m_pathTracerShader->setInt("u_samplesPerPixel", m_samplesPerPixel);
    
    updateSceneDataForShader(scene);
    renderGrid(camera);
    renderPrimitives(scene, camera);
    
    // Draw the fullscreen quad
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    
    m_pathTracerShader->unuse();
    m_drawCalls++;
    
    updateStats();
}

void Renderer::renderPrimitives(const Scene& scene, const Camera& camera) {
    if (!m_primitiveRenderer) {
        return;
    }
    
    m_primitiveRenderer->setCamera(camera);
    m_primitiveRenderer->setViewportSize(
        static_cast<int>(m_viewportSize.x),
        static_cast<int>(m_viewportSize.y)
    );
    
    const auto& objects = scene.getObjects();
    for (const auto& obj : objects) {
        if (!obj->isVisible()) {
            continue;
        }
        
        ObjectType type = obj->getType();
        Mat4 transform = obj->getTransform().getMatrix();
        Vec3 color = obj->getMaterial().color;
        bool isSelected = obj->isSelected();
        
        m_primitiveRenderer->renderPrimitive(type, transform, color, isSelected, false);
        m_drawCalls++;
    }
}


void Renderer::updateSceneDataForShader(const Scene& scene) {
    // Clear previous data
    m_sphereData.clear();
    m_planeData.clear();
    m_cubeData.clear();
    
    // Count objects by type
    size_t sphereCount = 0;
    size_t planeCount = 0;
    size_t cubeCount = 0;
    
    // First pass: count objects by type
    const auto& objects = scene.getObjects();
    for (const auto& obj : objects) {
        if (!obj->isVisible()) continue;
        
        switch (obj->getType()) {
            case ObjectType::Sphere: sphereCount++; break;
            case ObjectType::Plane: planeCount++; break;
            case ObjectType::Cube: cubeCount++; break;
            default: break;
        }
    }
    
    // Resize buffers
    m_sphereData.resize(sphereCount);
    m_planeData.resize(planeCount);
    m_cubeData.resize(cubeCount);
    
    // Second pass: populate data
    size_t sphereIdx = 0;
    size_t planeIdx = 0;
    size_t cubeIdx = 0;
    
    for (const auto& obj : objects) {
        if (!obj->isVisible()) continue;
        
        // Get common data
        IntersectionData data;
        obj->getIntersectionData(data);
        
        switch (obj->getType()) {
            case ObjectType::Sphere:
                if (sphereIdx < m_sphereData.size()) {
                    m_sphereData[sphereIdx++] = data;
                }
                break;
            case ObjectType::Plane:
                if (planeIdx < m_planeData.size()) {
                    m_planeData[planeIdx++] = data;
                }
                break;
            case ObjectType::Cube:
                if (cubeIdx < m_cubeData.size()) {
                    m_cubeData[cubeIdx++] = data;
                }
                break;
            default:
                break;
        }
    }
    
    // Upload sphere data
    for (size_t i = 0; i < m_sphereData.size(); i++) {
        const auto& sphere = m_sphereData[i];
        std::string base = "u_spheres[" + std::to_string(i) + "]";
        
        m_pathTracerShader->setVec3(base + ".center", sphere.position);
        m_pathTracerShader->setFloat(base + ".radius", sphere.scale.x);
        m_pathTracerShader->setVec3(base + ".color", sphere.color);
        m_pathTracerShader->setInt(base + ".materialType", sphere.materialType);
        m_pathTracerShader->setFloat(base + ".roughness", sphere.roughness);
        m_pathTracerShader->setFloat(base + ".ior", sphere.ior);
        m_pathTracerShader->setFloat(base + ".metalness", sphere.metalness);
        m_pathTracerShader->setVec3(base + ".emission", sphere.emission);
    }
    
    // Upload plane data
    for (size_t i = 0; i < m_planeData.size(); i++) {
        const auto& plane = m_planeData[i];
        std::string base = "u_planes[" + std::to_string(i) + "]";
        
        m_pathTracerShader->setVec3(base + ".point", plane.position);
        m_pathTracerShader->setVec3(base + ".normal", plane.normal);
        m_pathTracerShader->setVec3(base + ".color", plane.color);
        m_pathTracerShader->setInt(base + ".materialType", plane.materialType);
        m_pathTracerShader->setFloat(base + ".roughness", plane.roughness);
        m_pathTracerShader->setFloat(base + ".metalness", plane.metalness);
        m_pathTracerShader->setVec3(base + ".emission", plane.emission);
    }
    
    // Upload cube data (for intersection in the shader)
    for (size_t i = 0; i < m_cubeData.size(); i++) {
        const auto& cube = m_cubeData[i];
        std::string base = "u_cubes[" + std::to_string(i) + "]";
        
        m_pathTracerShader->setVec3(base + ".center", cube.position);
        m_pathTracerShader->setVec3(base + ".size", cube.scale);
        m_pathTracerShader->setVec3(base + ".color", cube.color);
        m_pathTracerShader->setInt(base + ".materialType", cube.materialType);
        m_pathTracerShader->setFloat(base + ".roughness", cube.roughness);
        m_pathTracerShader->setFloat(base + ".ior", cube.ior);
        m_pathTracerShader->setFloat(base + ".metalness", cube.metalness);
        m_pathTracerShader->setVec3(base + ".emission", cube.emission);
    }
    
    // Set counts
    m_pathTracerShader->setInt("u_numSpheres", static_cast<int>(m_sphereData.size()));
    m_pathTracerShader->setInt("u_numPlanes", static_cast<int>(m_planeData.size()));
    m_pathTracerShader->setInt("u_numCubes", static_cast<int>(m_cubeData.size()));
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
    if (!m_primitiveRenderer) {
        return;
    }
    
    // Update camera information
    m_primitiveRenderer->setCamera(camera);
    
    // Get object information
    ObjectType type = object.getType();
    Mat4 transform = object.getTransform().getMatrix();
    
    // Render wireframe with orange color and thicker lines
    m_primitiveRenderer->renderWireframe(type, transform, Vec3{1.0f, 0.5f, 0.0f}, 2.0f);
    
    m_drawCalls++;
}

void Renderer::renderHoverOutline(const Object& object, const Camera& camera) {
    if (!m_primitiveRenderer) {
        return;
    }
    
    // Update camera information
    m_primitiveRenderer->setCamera(camera);
    
    // Get object information
    ObjectType type = object.getType();
    Mat4 transform = object.getTransform().getMatrix();
    
    // Render wireframe with light gray color
    m_primitiveRenderer->renderWireframe(type, transform, Vec3{0.8f, 0.8f, 0.8f}, 1.5f);
    
    m_drawCalls++;
}

void Renderer::renderObjectWireframe(const Object& object) {
    switch (object.getType()) {
        case ObjectType::Sphere:
            renderSphereWireframe();
            break;
        case ObjectType::Cube:
            renderCubeWireframe();
            break;
        case ObjectType::Plane:
            renderPlaneWireframe();
            break;
        default:
            break;
    }
}

void Renderer::renderSphereWireframe() {
    const int segments = 16;
    const int rings = 12;
    
    std::vector<Vec3> vertices;
    std::vector<unsigned int> indices;
    
    // Generate sphere vertices
    for (int i = 0; i <= rings; ++i) {
        float phi = M_PI * float(i) / float(rings);
        for (int j = 0; j <= segments; ++j) {
            float theta = 2.0f * M_PI * float(j) / float(segments);
            
            float x = std::sin(phi) * std::cos(theta);
            float y = std::cos(phi);
            float z = std::sin(phi) * std::sin(theta);
            
            vertices.push_back(Vec3{x, y, z});
        }
    }
    
    // Generate indices for wireframe
    for (int i = 0; i < rings; ++i) {
        for (int j = 0; j < segments; ++j) {
            int curr = i * (segments + 1) + j;
            int next = curr + segments + 1;
            
            indices.push_back(curr);
            indices.push_back(curr + 1);
            
            indices.push_back(curr);
            indices.push_back(next);
        }
    }
    
    // Render wireframe
    unsigned int vao, vbo, ibo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vec3), vertices.data(), GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), (void*)0);
    glEnableVertexAttribArray(0);
    
    glDrawElements(GL_LINES, indices.size(), GL_UNSIGNED_INT, 0);
    
    glBindVertexArray(0);
    glDeleteBuffers(1, &ibo);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

void Renderer::renderCubeWireframe() {
    // Cube wireframe vertices
    Vec3 vertices[] = {
        {-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f},
        {0.5f,  0.5f, -0.5f}, {-0.5f,  0.5f, -0.5f},
        {-0.5f, -0.5f,  0.5f}, {0.5f, -0.5f,  0.5f},
        {0.5f,  0.5f,  0.5f}, {-0.5f,  0.5f,  0.5f}
    };
    
    unsigned int indices[] = {
        0,1, 1,2, 2,3, 3,0,  // Front face
        4,5, 5,6, 6,7, 7,4,  // Back face
        0,4, 1,5, 2,6, 3,7   // Connecting edges
    };
    
    // Render cube wireframe
    unsigned int vao, vbo, ibo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_DYNAMIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), (void*)0);
    glEnableVertexAttribArray(0);
    
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
    
    glBindVertexArray(0);
    glDeleteBuffers(1, &ibo);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

void Renderer::renderPlaneWireframe() {
    // Plane wireframe (grid pattern)
    const int gridSize = 5;
    std::vector<Vec3> vertices;
    
    for (int i = -gridSize; i <= gridSize; ++i) {
        float t = float(i) / float(gridSize);
        vertices.push_back(Vec3{-0.5f, 0, t * 0.5f});
        vertices.push_back(Vec3{0.5f, 0, t * 0.5f});
        vertices.push_back(Vec3{t * 0.5f, 0, -0.5f});
        vertices.push_back(Vec3{t * 0.5f, 0, 0.5f});
    }
    
    // Render plane wireframe
    unsigned int vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vec3), vertices.data(), GL_DYNAMIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), (void*)0);
    glEnableVertexAttribArray(0);
    
    glDrawArrays(GL_LINES, 0, vertices.size());
    
    glBindVertexArray(0);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
}

void Renderer::updateStats() {
    m_fps = Time::getFPS();
}

void Renderer::reloadShaders() {
    LOG_INFO("Reloading shaders...");
    
    // Reload via resource manager
    auto& rm = ResourceManager::instance();
    rm.clearShaders();
    
    m_pathTracerShader = rm.loadShader("pathtracer", "shaders/pathtracer.vert", "shaders/pathtracer.frag");
    if (m_pathTracerShader && m_pathTracerShader->isValid()) {
        LOG_INFO("Path tracer shader reloaded successfully");
    } else {
        LOG_ERROR("Failed to reload path tracer shader");
    }
    
    m_wireframeShader = rm.loadShader("wireframe", "shaders/wireframe.vert", "shaders/wireframe.frag");
    if (m_wireframeShader && m_wireframeShader->isValid()) {
        LOG_INFO("Wireframe shader reloaded successfully");
    } else {
        LOG_WARN("Failed to reload wireframe shader");
    }
    
    m_gridShader = rm.loadShader("grid", "shaders/grid.vert", "shaders/grid.frag");
    if (m_gridShader && m_gridShader->isValid()) {
        LOG_INFO("Grid shader reloaded successfully");
    } else {
        LOG_WARN("Failed to reload grid shader");
    }
}