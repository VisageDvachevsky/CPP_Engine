#pragma once

class Framebuffer {
public:
    Framebuffer(int width, int height);
    ~Framebuffer();
    
    void bind() const;
    void unbind() const;
    
    void resize(int width, int height);
    
    unsigned int getFramebuffer() const { return m_framebuffer; }
    unsigned int getColorTexture() const { return m_colorTexture; }
    unsigned int getDepthTexture() const { return m_depthTexture; }
    
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

private:
    void createFramebuffer();
    void deleteFramebuffer();
    
    unsigned int m_framebuffer = 0;
    unsigned int m_colorTexture = 0;
    unsigned int m_depthTexture = 0;
    
    int m_width, m_height;
};