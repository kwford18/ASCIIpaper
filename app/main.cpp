#include <iostream>
#include "engine/window.h"
#include "engine/renderer.h"
#include "engine/grid.h"
#include "engine/timer.h"
#include "worlds/aquarium.h"

int main(int argc, char* argv[]) {
    // std::cout << "Starting wall-aquarium..." << std::endl;

    // Create a 800x600 window
    Aquarium::Engine::Window window("wall-aquarium", 800, 600);
    if (!window.Initialize()) {
        std::cerr << "Failed to initialize the engine window." << std::endl;
        return -1;
    }

    // Initialize the renderer with the created window
    Aquarium::Engine::Renderer renderer(window.GetNativeWindow());
    if (!renderer.Initialize()) {
        std::cerr << "Failed to initialize the renderer." << std::endl;
        return -1;
    }

    /* 
     * Create the static Character Grid
     * Since the window is 800x600 and cells are scaled to 16x16 pixels:
     * Width: 800 / 16 = 50 columns
     * Height: 600 / 16 = 37 rows
    */
    Aquarium::Engine::CharacterGrid grid(50, 37);
    Aquarium::Worlds::AquariumScene scene(50, 37);

    // Create a timer to lock the engine at 60 FPS
    Aquarium::Engine::Timer timer(60);
    timer.Start();

    // Main Engine Loop
    while (!window.ShouldClose()) {
        // Process Input/OS Events
        window.PollEvents();

        // Update Scene Logic
        scene.Update(timer.GetDeltaTime());

        // Draw Scene to Grid
        scene.Draw(grid);

        // Render the grid to the screen
        renderer.DrawGrid(grid);
        
        // Wait for next frame
        timer.Tick();
    }

    // std::cout << "Shutting down gracefully." << std::endl;
    return 0;
}