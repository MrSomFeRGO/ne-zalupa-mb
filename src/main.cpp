//
// Created by mrsomfergo on 13.07.2025.
//

#include "core/Application.h"
#include <iostream>
#include <stdexcept>

int main(int argc, char* argv[]) {
    std::cout << "========================================" << std::endl;
    std::cout << "    VoxelEngine - OpenGL Edition" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Built with:" << std::endl;
    std::cout << "  - OpenGL 4.5 Core (fallback to 3.3)" << std::endl;
    std::cout << "  - SDL3" << std::endl;
    std::cout << "  - GLM" << std::endl;
    std::cout << "  - FastNoiseLite" << std::endl;
    std::cout << "  - STB Image" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Fixed: OpenGL threading issues (no more SIGSEGV!)" << std::endl;
    std::cout << "========================================" << std::endl;

    try {
        Application app;

        if (!app.Initialize("VoxelEngine - OpenGL Edition", 1280, 720)) {
            std::cerr << "Failed to initialize application!" << std::endl;
            return -1;
        }

        std::cout << "Application initialized successfully!" << std::endl;
        std::cout << "Starting main loop..." << std::endl;

        app.Run();
        app.Shutdown();

        std::cout << "Application shutdown complete." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        return -1;
    } catch (...) {
        std::cerr << "Unknown fatal error occurred!" << std::endl;
        return -1;
    }

    return 0;
}