#include <iostream>
#include <SDL3/SDL_main.h>


#include "engine/window.h"
#include "engine/renderer.h"
#include "engine/grid.h"
#include "engine/timer.h"
#include "engine/config.h"
#include "engine/scene_manager.h"

#include "worlds/aquarium.h"
#include "worlds/city.h"

#include "platform/desktop.h"

int main(int argc, char* argv[]) {
    // Determine the absolute path to the executable's folder
    std::string configPath = "config.ini";
    
    const char* basePath = SDL_GetBasePath(); 
    
    if (basePath) {
        // basePath includes the trailing slash (e.g., "C:\...\build\Debug\")
        configPath = std::string(basePath) + "config.ini";
    }

    // Load Configuration using the absolute path
    ASCIIpaper::Engine::Config config;
    if (config.Load(configPath)) {
        std::cout << "Loaded config.ini successfully.\n";
    } else {
        std::cout << "No config.ini found. Using default engine settings.\n";
    }

    // Extract variables with defaults
    std::string activeScene = config.GetString("scene", "aquarium");

    // Aquarium config variables
    int targetFps = config.GetInt("target_fps", 30);
    int fishCount = config.GetInt("fish_count", 6);
    int bubbleCount = config.GetInt("bubble_count", 15);
    int jellyCount = config.GetInt("jellyfish_count", 3);
    
    // City config variables
    int carCount = config.GetInt("car_count", 12);
    int starCount = config.GetInt("star_count", 40);

    // Weather config variables
    std::string weatherMode = config.GetString("weather", "rain");
    
    // System sync config variables
    bool systemSync = config.GetBool("system_sync", false);

    // Initialize Engine
    ASCIIpaper::Engine::Window window("ASCIIpaper", 1920, 1080);
    if (!window.Initialize()) {
        return -1;
    }
    ASCIIpaper::Platform::AttachToDesktop(window.GetNativeWindow());

    ASCIIpaper::Engine::Renderer renderer(window.GetNativeWindow());
    if (!renderer.Initialize()) {
        return -1;
    }

    ASCIIpaper::Engine::CharacterGrid grid(120, 67); // 1920/16 and 1080/16
    
    // Initialize the Scene Manager
    ASCIIpaper::Engine::SceneManager sceneManager;

    // Select the scene based on the config file
    if (activeScene == "city") {
        sceneManager.ChangeScene(std::make_unique<ASCIIpaper::Worlds::CityScene>(
            120, 67, carCount, starCount, weatherMode, systemSync
        ));
    } else {
        // Fallback to the aquarium for any other value
        sceneManager.ChangeScene(std::make_unique<ASCIIpaper::Worlds::AquariumScene>(
            120, 67, fishCount, bubbleCount, jellyCount
        ));
    }

    // Apply adjustable framerate
    ASCIIpaper::Engine::Timer timer(targetFps);
    timer.Start();

    // Main Engine Loop
    while (!window.ShouldClose() && !ASCIIpaper::Platform::ShouldQuit()) {
        // Process Input/OS Events
        window.PollEvents();

        // Update Scene Logic
        sceneManager.Update(timer.GetDeltaTime());

        // Draw Scene to Grid
        sceneManager.Draw(grid);

        // Render the grid to the screen
        renderer.DrawGrid(grid);

        // Wait for next frame
        timer.Tick();
    }

    ASCIIpaper::Platform::ShutdownDesktopIntegration();

    return 0;
}