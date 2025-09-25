#include "core/Application.h"
#include "core/Logger.h"
#include <iostream>

int main() {
    try {
        Logger::init();
        LOG_INFO("Starting MiniGPU Engine...");
        
        Application app;
        app.run();
        
        LOG_INFO("Engine shutdown complete.");
    } catch (const std::exception& e) {
        LOG_ERROR("Fatal error: {}", e.what());
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}