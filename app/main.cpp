#include <iostream>
#include "engine/window.h"
#include "engine/renderer.h"
#include "engine/grid.h"
#include "engine/timer.h"
#include "engine/config.h"
#include "worlds/aquarium.h"
#include "platform/desktop.h"

int main(int argc, char* argv[]) {
    // Load Configuration
    Aquarium::Engine::Config config;
    if (config.Load("config.ini")) {
        std::cout << "Loaded config.ini successfully.\n";
    } else {
        std::cout << "No config.ini found. Using default engine settings.\n";
    }

    // Extract variables with defaults
    int targetFps = config.GetInt("target_fps", 60);
    int fishCount = config.GetInt("fish_count", 6);
    int bubbleCount = config.GetInt("bubble_count", 15);
    int jellyCount = config.GetInt("jellyfish_count", 3);

    // Initialize Engine
    Aquarium::Engine::Window window("wall-aquarium", 1920, 1080);
    if (!window.Initialize()) {
        return -1;
    }
    Aquarium::Platform::AttachToDesktop(window.GetNativeWindow());

    Aquarium::Engine::Renderer renderer(window.GetNativeWindow());
    if (!renderer.Initialize()) {
        return -1;
    }

    // Use our config variables!
    Aquarium::Engine::CharacterGrid grid(120, 67); // 1920/16 and 1080/16
    Aquarium::Worlds::AquariumScene scene(120, 67, fishCount, bubbleCount, jellyCount);

    /* 
     * Create the Character Grid, sized to match the actual window.
     * Window::Initialize() sizes the window to the primary display's
     * real resolution (needed so it lines up with the WorkerW area it gets
     * attached to). Computed to grid dimensions from the real size
     * instead of assuming a fixed window size.
     * SDL_RenderDebugText is an 8x8 pixel font, scaled 2x by the renderer,
     * so each grid cell is 16x16 pixels.
    */

    // Apply adjustable framerate
    Aquarium::Engine::Timer timer(targetFps);
    timer.Start();

    // Main Engine Loop
    while (!window.ShouldClose() && !Aquarium::Platform::ShouldQuit()) {
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

    Aquarium::Platform::ShutdownDesktopIntegration();

    // std::cout << "Shutting down gracefully." << std::endl;
    return 0;
}