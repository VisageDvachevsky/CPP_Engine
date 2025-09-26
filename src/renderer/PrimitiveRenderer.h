#pragma once

#include "math/Vec3.h"
#include "math/Mat4.h"
#include <vector>
#include <memory>

class Shader;
class Camera;
enum class ObjectType;

class PrimitiveRenderer {
public:
    PrimitiveRenderer();
    ~PrimitiveRenderer();
    
    // Initialize geometry buffers
    void initialize();
    
    // Render primitive with specified transform and color
    void renderPrimitive(ObjectType type, const Mat4& transform, const Vec3& color, 
                        bool isSelected = false, bool isHovered = false);
    
    // Render wireframe version of a primitive
    void renderWireframe(ObjectType type, const Mat4& transform, const Vec3& color,
                        float lineWidth = 1.0f);
    
    // Set rendering parameters
    void setCamera(const Camera& camera);
    void setViewportSize(int width, int height);
    
private:
    // Geometry generation functions
    void createSphereGeometry(int segments = 32, int rings = 16);
    void createCubeGeometry();
    void createPlaneGeometry();
    
    // Helpers
    void setupShaders();
    
    // Rendering state
    struct GeometryBuffers {
        unsigned int vao = 0;
        unsigned int vbo = 0;
        unsigned int ibo = 0;
        int indexCount = 0;
    };
    
    GeometryBuffers m_sphereBuffers;
    GeometryBuffers m_cubeBuffers;
    GeometryBuffers m_planeBuffers;
    
    // Shaders
    std::shared_ptr<Shader> m_solidShader;
    std::shared_ptr<Shader> m_wireframeShader;
    
    // Camera matrices
    Mat4 m_viewMatrix;
    Mat4 m_projMatrix;
    int m_viewportWidth = 1280;
    int m_viewportHeight = 720;
    
    // Selection/hover colors
    Vec3 m_selectedColor = Vec3(1.0f, 0.5f, 0.0f); // Orange
    Vec3 m_hoveredColor = Vec3(0.8f, 0.8f, 0.8f);  // Light gray
    
    // State tracking
    bool m_initialized = false;
};