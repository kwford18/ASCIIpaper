#include <iostream>
#include "engine/window.h"
#include "engine/renderer.h"
#include "engine/grid.h"
#include "engine/timer.h"
#include "worlds/aquarium.h"
#include "platform/desktop.h"

int main(int argc, char* argv[]) {
    // std::cout << "Starting wall-aquarium..." << std::endl;

    // Create a 800x600 window
    Aquarium::Engine::Window window("wall-aquarium", 800, 600);
    if (!window.Initialize()) {
        std::cerr << "Failed to initialize the engine window." << std::endl;
        return -1;
    }

    // Send to background on Windows
    Aquarium::Platform::AttachToDesktop(window.GetNativeWindow());

    // Initialize the renderer with the created window
    Aquarium::Engine::Renderer renderer(window.GetNativeWindow());
    if (!renderer.Initialize()) {
        std::cerr << "Failed to initialize the renderer." << std::endl;
        return -1;
    }

    /* 
     * Create the Character Grid, sized to match the actual window.
     * Window::Initialize() sizes the window to the primary display's
     * real resolution (needed so it lines up with the WorkerW area it gets
     * attached to). Computed to grid dimensions from the real size
     * instead of assuming a fixed window size.
     * SDL_RenderDebugText is an 8x8 pixel font, scaled 2x by the renderer,
     * so each grid cell is 16x16 pixels.
    */
    const int cellSize = 16;
    const int cols = window.GetWidth() / cellSize;
    const int rows = window.GetHeight() / cellSize;
    Aquarium::Engine::CharacterGrid grid(cols, rows);
    Aquarium::Worlds::AquariumScene scene(cols, rows);

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