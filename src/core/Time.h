#pragma once

class Time {
public:
    static void init();
    static void update(float deltaTime);
    
    static float getDeltaTime() { return s_deltaTime; }
    static float getTime() { return s_time; }
    static float getFPS() { return s_fps; }
    static int getFrameCount() { return s_frameCount; }
    
private:
    static float s_deltaTime;
    static float s_time;
    static float s_fps;
    static int s_frameCount;
    static float s_fpsTimer;
    static int s_fpsCounter;
};