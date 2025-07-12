//
// Created by mrsomfergo on 12.07.2025.
//

#include "core/Application.h"
#include <iostream>
#include <stdexcept>

int main(int argc, char* argv[]) {
    try {
        Application app;

        if (!app.Initialize("Voxel Engine", 1280, 720)) {
            std::cerr << "Failed to initialize application!" << std::endl;
            return -1;
        }

        app.Run();
        app.Shutdown();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }

    return 0;
}
