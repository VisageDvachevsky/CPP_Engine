#pragma once

#include <string>

class Texture {
public:
    Texture();
    ~Texture();
    
    bool loadFromFile(const std::string& path);
    
    void bind(unsigned int slot = 0) const;
    void unbind() const;
    
    unsigned int getID() const { return m_id; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    int getChannels() const { return m_channels; }

private:
    unsigned int m_id = 0;
    int m_width = 0;
    int m_height = 0;
    int m_channels = 0;
};