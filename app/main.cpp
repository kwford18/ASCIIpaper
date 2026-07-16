#include <iostream>
#include "engine/window.h"
#include "engine/renderer.h"
#include "engine/grid.h"

int main(int argc, char* argv[]) {
    std::cout << "Starting wall-aquarium..." << std::endl;

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

    // --- DRAW A TEST SCENE INTO THE GRID ---
    
    // Draw a basic border
    for (int x = 0; x < grid.GetWidth(); ++x) {
        grid.SetCell(x, 0, '#');
        grid.SetCell(x, grid.GetHeight() - 1, '#');
    }
    for (int y = 0; y < grid.GetHeight(); ++y) {
        grid.SetCell(0, y, '#');
        grid.SetCell(grid.GetWidth() - 1, y, '#');
    }
    
    // Draw simple fish shapes
    grid.SetCell(23, 18, '>');
    grid.SetCell(24, 18, '<');
    grid.SetCell(25, 18, '>');
    grid.SetCell(26, 18, 'O');

    // Draw some bubbles
    grid.SetCell(27, 16, 'o');
    grid.SetCell(28, 14, 'O');
    grid.SetCell(27, 11, 'o');

    // Main Engine Loop
    while (!window.ShouldClose()) {
        // Process Input/OS Events
        window.PollEvents();

        // Render the grid to the screen
        renderer.DrawGrid(grid);
        
        // Temporarily delay to prevent 100% CPU usage until we implement a fixed timestep
        SDL_Delay(16); 
    }

    std::cout << "Shutting down gracefully." << std::endl;
    return 0;
}