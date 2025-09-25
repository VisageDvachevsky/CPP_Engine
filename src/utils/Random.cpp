#include "Random.h"
#include <chrono>

thread_local std::mt19937 Random::s_generator{
    static_cast<unsigned int>(std::chrono::steady_clock::now().time_since_epoch().count())
};

thread_local std::uniform_real_distribution<float> Random::s_realDist{0.0f, 1.0f};

void Random::seed(unsigned int seed) {
    s_generator.seed(seed);
}

float Random::uniform() {
    return s_realDist(s_generator);
}

float Random::uniform(float min, float max) {
    return min + (max - min) * uniform();
}

int Random::uniformInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(s_generator);
}