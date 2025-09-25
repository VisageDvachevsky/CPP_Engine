#include "Time.h"

float Time::s_deltaTime = 0.0f;
float Time::s_time = 0.0f;
float Time::s_fps = 0.0f;
int Time::s_frameCount = 0;
float Time::s_fpsTimer = 0.0f;
int Time::s_fpsCounter = 0;

void Time::init() {
    s_deltaTime = 0.0f;
    s_time = 0.0f;
    s_fps = 0.0f;
    s_frameCount = 0;
    s_fpsTimer = 0.0f;
    s_fpsCounter = 0;
}

void Time::update(float deltaTime) {
    s_deltaTime = deltaTime;
    s_time += deltaTime;
    s_frameCount++;
    
    // Calculate FPS
    s_fpsTimer += deltaTime;
    s_fpsCounter++;
    
    if (s_fpsTimer >= 0.5f) { // Update FPS every 0.5 seconds
        s_fps = s_fpsCounter / s_fpsTimer;
        s_fpsTimer = 0.0f;
        s_fpsCounter = 0;
    }
}