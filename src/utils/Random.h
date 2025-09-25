#pragma once

#include <random>

class Random {
public:
    static void seed(unsigned int seed);
    static float uniform(); // 0.0 to 1.0
    static float uniform(float min, float max);
    static int uniformInt(int min, int max);

private:
    static thread_local std::mt19937 s_generator;
    static thread_local std::uniform_real_distribution<float> s_realDist;
};