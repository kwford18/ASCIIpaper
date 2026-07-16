#include <iostream>
#include "engine/Window.h"

int main(int argc, char* argv[]) {
    std::cout << "Starting wall-aquarium..." << std::endl;

    // Create a 800x600 window
    Aquarium::Engine::Window window("wall-aquarium", 800, 600);

    if (!window.Initialize()) {
        std::cerr << "Failed to initialize the engine window." << std::endl;
        return -1;
    }

    // Main Engine Loop
    while (!window.ShouldClose()) {
        // Process Input/OS Events
        window.PollEvents();

        // Update Scene (TODO)
        
        // Render (TODO)
        
        // Temporarily delay to prevent 100% CPU usage until we implement a fixed timestep
        SDL_Delay(16); 
    }

    std::cout << "Shutting down gracefully." << std::endl;
    return 0;
}