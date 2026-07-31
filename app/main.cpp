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
    // Get configuration file
    std::string configPath = "config.ini";
    const char* basePath = SDL_GetBasePath(); 
    
    if (basePath) {
        configPath = std::string(basePath) + "config.ini";
    }

    ASCIIpaper::Engine::Config config;
    if (config.Load(configPath)) {
        std::cout << "Loaded config.ini successfully.\n";
    } else {
        std::cout << "No config.ini found. Using default engine settings.\n";
    }

    // Create window and attach to background
    ASCIIpaper::Engine::Window window("ASCIIpaper", 1920, 1080);
    if (!window.Initialize()) return -1;
    
    ASCIIpaper::Platform::AttachToDesktop(window.GetNativeWindow());

    ASCIIpaper::Engine::Renderer renderer(window.GetNativeWindow());
    if (!renderer.Initialize()) return -1;

    ASCIIpaper::Engine::CharacterGrid grid(120, 67); 
    ASCIIpaper::Engine::SceneManager sceneManager;

    /*
     * We wrap the scene creation inside a lambda function.
     * This allows us to instantly wipe the current scene and load a brand
     * new one at runtime whenever the system tray modifies the config file!
     */
    auto loadScene = [&]() {
        // Load config variables
        std::string activeScene = config.GetString("scene", "aquarium");
        int fishCount = config.GetInt("fish_count", 6);
        int bubbleCount = config.GetInt("bubble_count", 15);
        int jellyCount = config.GetInt("jellyfish_count", 3);
        int carCount = config.GetInt("car_count", 12);
        int starCount = config.GetInt("star_count", 40);
        std::string weatherMode = config.GetString("weather", "rain");
        bool systemSync = config.GetBool("system_sync", false);

        // Load selected scene
        if (activeScene == "city") {
            sceneManager.ChangeScene(std::make_unique<ASCIIpaper::Worlds::CityScene>(
                120, 67, carCount, starCount, weatherMode, systemSync
            ));
        } else {
            sceneManager.ChangeScene(std::make_unique<ASCIIpaper::Worlds::AquariumScene>(
                120, 67, fishCount, bubbleCount, jellyCount, systemSync
            ));
        }
    };

    // Load the initial scene on startup
    loadScene();

    // Set target FPS
    int targetFps = config.GetInt("target_fps", 30);
    ASCIIpaper::Engine::Timer timer(targetFps);
    timer.Start();

    // Main Engine Loop
    while (!window.ShouldClose() && !ASCIIpaper::Platform::ShouldQuit()) {
        
        // Hot Reload the scene
        if (ASCIIpaper::Platform::HasConfigChanged()) {
            ASCIIpaper::Platform::ClearConfigChanged();
            config.Load(configPath); // Reload the fresh file from disk
            loadScene();             // Rebuild the scene
        }

        window.PollEvents();
        sceneManager.Update(timer.GetDeltaTime());
        sceneManager.Draw(grid);
        renderer.DrawGrid(grid);
        timer.Tick();
    }

    ASCIIpaper::Platform::ShutdownDesktopIntegration();
    return 0;
}