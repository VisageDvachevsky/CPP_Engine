#pragma once

class Renderer;

class StatusBar {
public:
    StatusBar() = default;
    ~StatusBar() = default;
    
    void show(const Renderer& renderer);
};