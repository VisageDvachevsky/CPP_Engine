#include "PrimitiveRenderer.h"
#include "scene/Object.h"
#include "scene/Camera.h"
#include "Shader.h"
#include "utils/ResourceManager.h"
#include "core/Logger.h"
#include <glad/glad.h>
#include <cmath>

PrimitiveRenderer::PrimitiveRenderer() {
    LOG_INFO("PrimitiveRenderer created");
}

PrimitiveRenderer::~PrimitiveRenderer() {
    // Clean up OpenGL resources
    if (m_sphereBuffers.vao) {
        glDeleteVertexArrays(1, &m_sphereBuffers.vao);
        glDeleteBuffers(1, &m_sphereBuffers.vbo);
        glDeleteBuffers(1, &m_sphereBuffers.ibo);
    }
    
    if (m_cubeBuffers.vao) {
        glDeleteVertexArrays(1, &m_cubeBuffers.vao);
        glDeleteBuffers(1, &m_cubeBuffers.vbo);
        glDeleteBuffers(1, &m_cubeBuffers.ibo);
    }
    
    if (m_planeBuffers.vao) {
        glDeleteVertexArrays(1, &m_planeBuffers.vao);
        glDeleteBuffers(1, &m_planeBuffers.vbo);
        glDeleteBuffers(1, &m_planeBuffers.ibo);
    }
    
    LOG_INFO("PrimitiveRenderer destroyed");
}

void PrimitiveRenderer::initialize() {
    if (m_initialized) return;
    
    // Set up shaders
    setupShaders();
    
    // Create geometry for primitives
    createSphereGeometry();
    createCubeGeometry();
    createPlaneGeometry();
    
    m_initialized = true;
    LOG_INFO("PrimitiveRenderer initialized");
}

void PrimitiveRenderer::setupShaders() {
    auto& rm = ResourceManager::instance();
    
    // Load shaders for solid rendering
    m_solidShader = rm.loadShader("primitive_solid", 
                                 "shaders/primitive/solid.vert", 
                                 "shaders/primitive/solid.frag");
    
    // Load shaders for wireframe rendering
    m_wireframeShader = rm.loadShader("primitive_wireframe",
                                    "shaders/primitive/wireframe.vert",
                                    "shaders/primitive/wireframe.frag");
    
    if (!m_solidShader || !m_wireframeShader) {
        // Use default shaders if custom ones not found
        m_solidShader = rm.loadShader("primitive_solid", 
                                     "shaders/wireframe.vert", 
                                     "shaders/wireframe.frag");
        m_wireframeShader = m_solidShader; // Use same shader for both
        
        LOG_WARN("Using fallback shaders for primitive rendering");
    }
}

void PrimitiveRenderer::createSphereGeometry(int segments, int rings) {
    std::vector<Vec3> vertices;
    std::vector<unsigned int> indices;
    
    // Generate sphere vertices
    for (int r = 0; r <= rings; r++) {
        float phi = static_cast<float>(M_PI) * float(r) / float(rings);
        float sinPhi = std::sin(phi);
        float cosPhi = std::cos(phi);
        
        for (int s = 0; s <= segments; s++) {
            float theta = 2.0f * static_cast<float>(M_PI) * float(s) / float(segments);
            float sinTheta = std::sin(theta);
            float cosTheta = std::cos(theta);
            
            // Vertex position
            Vec3 position(
                sinPhi * cosTheta, // x
                cosPhi,            // y
                sinPhi * sinTheta  // z
            );
            
            // Add vertex
            vertices.push_back(position);
            
            // Generate indices for triangle strips
            if (r < rings && s < segments) {
                int current = r * (segments + 1) + s;
                int next = current + segments + 1;
                
                // First triangle
                indices.push_back(current);
                indices.push_back(current + 1);
                indices.push_back(next);
                
                // Second triangle
                indices.push_back(current + 1);
                indices.push_back(next + 1);
                indices.push_back(next);
            }
        }
    }
    
    // Create OpenGL buffers
    glGenVertexArrays(1, &m_sphereBuffers.vao);
    glGenBuffers(1, &m_sphereBuffers.vbo);
    glGenBuffers(1, &m_sphereBuffers.ibo);
    
    glBindVertexArray(m_sphereBuffers.vao);
    
    // Vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_sphereBuffers.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vec3), vertices.data(), GL_STATIC_DRAW);
    
    // Element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_sphereBuffers.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
    
    m_sphereBuffers.indexCount = indices.size();
    LOG_DEBUG("Sphere geometry created: {} vertices, {} indices", vertices.size(), indices.size());
}

void PrimitiveRenderer::createCubeGeometry() {
    // Cube vertices (position only)
    std::vector<Vec3> vertices = {
        // Front face
        {-0.5f, -0.5f,  0.5f},
        { 0.5f, -0.5f,  0.5f},
        { 0.5f,  0.5f,  0.5f},
        {-0.5f,  0.5f,  0.5f},
        
        // Back face
        {-0.5f, -0.5f, -0.5f},
        { 0.5f, -0.5f, -0.5f},
        { 0.5f,  0.5f, -0.5f},
        {-0.5f,  0.5f, -0.5f}
    };
    
    // Cube indices (triangles)
    std::vector<unsigned int> indices = {
        // Front
        0, 1, 2,
        0, 2, 3,
        
        // Right
        1, 5, 6,
        1, 6, 2,
        
        // Back
        5, 4, 7,
        5, 7, 6,
        
        // Left
        4, 0, 3,
        4, 3, 7,
        
        // Top
        3, 2, 6,
        3, 6, 7,
        
        // Bottom
        4, 5, 1,
        4, 1, 0
    };
    
    // Create OpenGL buffers
    glGenVertexArrays(1, &m_cubeBuffers.vao);
    glGenBuffers(1, &m_cubeBuffers.vbo);
    glGenBuffers(1, &m_cubeBuffers.ibo);
    
    glBindVertexArray(m_cubeBuffers.vao);
    
    // Vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_cubeBuffers.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vec3), vertices.data(), GL_STATIC_DRAW);
    
    // Element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_cubeBuffers.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
    
    m_cubeBuffers.indexCount = indices.size();
    LOG_DEBUG("Cube geometry created: {} vertices, {} indices", vertices.size(), indices.size());
}

void PrimitiveRenderer::createPlaneGeometry() {
    // Plane vertices (position only) - XZ plane centered at origin
    std::vector<Vec3> vertices = {
        {-0.5f, 0.0f, -0.5f},
        { 0.5f, 0.0f, -0.5f},
        { 0.5f, 0.0f,  0.5f},
        {-0.5f, 0.0f,  0.5f}
    };
    
    // Plane indices (two triangles)
    std::vector<unsigned int> indices = {
        0, 1, 2,
        0, 2, 3
    };
    
    // Create OpenGL buffers
    glGenVertexArrays(1, &m_planeBuffers.vao);
    glGenBuffers(1, &m_planeBuffers.vbo);
    glGenBuffers(1, &m_planeBuffers.ibo);
    
    glBindVertexArray(m_planeBuffers.vao);
    
    // Vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_planeBuffers.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vec3), vertices.data(), GL_STATIC_DRAW);
    
    // Element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_planeBuffers.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
    
    m_planeBuffers.indexCount = indices.size();
    LOG_DEBUG("Plane geometry created: {} vertices, {} indices", vertices.size(), indices.size());
}

void PrimitiveRenderer::setCamera(const Camera& camera) {
    m_viewMatrix = camera.getViewMatrix();
    m_projMatrix = camera.getProjectionMatrix(
        static_cast<float>(m_viewportWidth) / static_cast<float>(m_viewportHeight)
    );
}

void PrimitiveRenderer::setViewportSize(int width, int height) {
    m_viewportWidth = width;
    m_viewportHeight = height;
}

void PrimitiveRenderer::renderPrimitive(ObjectType type, const Mat4& transform, 
                                     const Vec3& color, bool isSelected, bool isHovered) {
    if (!m_initialized) {
        LOG_ERROR("PrimitiveRenderer not initialized");
        return;
    }
    
    if (!m_solidShader || !m_solidShader->isValid()) {
        LOG_ERROR("Invalid shader for primitive rendering");
        return;
    }
    
    m_solidShader->use();
    
    // Set up matrices
    Mat4 mvp = m_projMatrix * m_viewMatrix * transform;
    m_solidShader->setMat4("u_mvp", mvp);
    
    // Set up color (apply selection/hover highlights if needed)
    Vec3 finalColor = color;
    
    if (isSelected) {
        // Blend with selection color to make it brighter/highlighted
        finalColor = finalColor * 0.7f + m_selectedColor * 0.3f;
    } else if (isHovered) {
        // Blend with hover color for subtle highlight
        finalColor = finalColor * 0.8f + m_hoveredColor * 0.2f;
    }
    
    m_solidShader->setVec3("u_color", finalColor);
    
    // Select and render the appropriate geometry
    const GeometryBuffers* buffers = nullptr;
    
    switch (type) {
        case ObjectType::Sphere:
            buffers = &m_sphereBuffers;
            break;
        case ObjectType::Cube:
            buffers = &m_cubeBuffers;
            break;
        case ObjectType::Plane:
            buffers = &m_planeBuffers;
            break;
        default:
            LOG_ERROR("Unknown primitive type");
            m_solidShader->unuse();
            return;
    }
    
    glBindVertexArray(buffers->vao);
    glDrawElements(GL_TRIANGLES, buffers->indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    
    m_solidShader->unuse();
}

void PrimitiveRenderer::renderWireframe(ObjectType type, const Mat4& transform, 
                                      const Vec3& color, float lineWidth) {
    if (!m_initialized) {
        LOG_ERROR("PrimitiveRenderer not initialized");
        return;
    }
    
    if (!m_wireframeShader || !m_wireframeShader->isValid()) {
        LOG_ERROR("Invalid shader for wireframe rendering");
        return;
    }
    
    // Enable wireframe mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(lineWidth);
    
    m_wireframeShader->use();
    
    // Set up matrices
    Mat4 mvp = m_projMatrix * m_viewMatrix * transform;
    m_wireframeShader->setMat4("u_mvp", mvp);
    
    // Set up color
    m_wireframeShader->setVec3("u_color", color);
    
    // Select and render the appropriate geometry
    const GeometryBuffers* buffers = nullptr;
    
    switch (type) {
        case ObjectType::Sphere:
            buffers = &m_sphereBuffers;
            break;
        case ObjectType::Cube:
            buffers = &m_cubeBuffers;
            break;
        case ObjectType::Plane:
            buffers = &m_planeBuffers;
            break;
        default:
            LOG_ERROR("Unknown primitive type");
            m_wireframeShader->unuse();
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Restore polygon mode
            return;
    }
    
    glBindVertexArray(buffers->vao);
    glDrawElements(GL_TRIANGLES, buffers->indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    
    m_wireframeShader->unuse();
    
    // Restore polygon mode
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}