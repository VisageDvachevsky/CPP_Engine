#pragma once

#include "Shader.h"
#include "math/Vec2.h"
#include <memory>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Scene;
class Camera;
class Object;

class Renderer {
public:
    Renderer();
    ~Renderer();
    
    void render(const Scene& scene, const Camera& camera);
    void clear();
    void reloadShaders();
    
    // Viewport management
    void setViewportSize(int width, int height) { m_viewportSize = Vec2{static_cast<float>(width), static_cast<float>(height)}; }
    Vec2 getViewportSize() const { return m_viewportSize; }
    
    // Path tracer settings
    void setSamplesPerPixel(int spp) { m_samplesPerPixel = spp; }
    void setMaxBounces(int bounces) { m_maxBounces = bounces; }
    
    int getSamplesPerPixel() const { return m_samplesPerPixel; }
    int getMaxBounces() const { return m_maxBounces; }
    
    // Statistics
    float getFPS() const { return m_fps; }
    int getDrawCalls() const { return m_drawCalls; }
    
    // Selection rendering
    void renderSelectionOutline(const Object& object, const Camera& camera);
    void renderHoverOutline(const Object& object, const Camera& camera);

private:
    void createQuad();
    void createGrid();
    void updateUniforms(const Scene& scene, const Camera& camera);
    void updateStats();
    void renderGrid(const Camera& camera);
    void renderObjectWireframe(const Object& object);
    
    std::unique_ptr<Shader> m_pathTracerShader;
    std::unique_ptr<Shader> m_wireframeShader;
    std::unique_ptr<Shader> m_gridShader;
    
    unsigned int m_quadVAO = 0, m_quadVBO = 0;
    unsigned int m_gridVAO = 0, m_gridVBO = 0, m_gridIBO = 0;
    
    // Viewport
    Vec2 m_viewportSize{1920, 1080};
    
    // Render settings
    int m_samplesPerPixel = 16;
    int m_maxBounces = 8;
    
    // Stats
    float m_fps = 0.0f;
    int m_drawCalls = 0;
    float m_frameTime = 0.0f;
    int m_frameCount = 0;
};