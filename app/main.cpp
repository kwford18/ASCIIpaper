#include <SDL3/SDL_main.h>
#include <format>
#include <fstream>
#include <random>
#include <string>
#include <vector>

#include "engine/config.h"
#include "engine/grid.h"
#include "engine/logger.h"
#include "engine/renderer.h"
#include "engine/scene_manager.h"
#include "engine/timer.h"
#include "engine/window.h"

#include "platform/desktop.h"
#include "worlds/aquarium.h"
#include "worlds/city.h"

// Helper function to safely overwrite a specific key in config.ini
void UpdateConfigFile(const std::string& configPath, const std::string& key,
                      const std::string& value) {
    std::vector<std::string> lines;
    std::ifstream inFile(configPath);
    bool found = false;
    std::string line;

    if (inFile.is_open()) {
        while (std::getline(inFile, line)) {
            if (line.find(key) == 0 && (line[key.length()] == ' ' || line[key.length()] == '=')) {
                lines.push_back(std::format("{} = {}", key, value));
                found = true;
            } else {
                lines.push_back(line);
            }
        }
        inFile.close();
    }

    // If the key wasn't in the file at all, append it to the bottom
    if (!found) {
        lines.push_back(std::format("{} = {}", key, value));
    }

    std::ofstream outFile(configPath);
    for (const auto& l : lines) {
        outFile << l << "\n";
    }
}

int main(int argc, char* argv[]) {
    // Get configuration file path
    std::string configPath = "config.ini";
    const char* basePath = SDL_GetBasePath();

    if (basePath) {
        configPath = std::string(basePath) + "config.ini";
    }

    // CLI Mode
    // If the user runs ./ASCIIpaper set [key] [value], update the config and exit immediately
    if (argc >= 3 && std::string(argv[1]) == "set") {
        std::string key = argv[2];
        std::string value = (argc >= 4) ? argv[3] : "";

        UpdateConfigFile(configPath, key, value);
        ASCII_COUT << "Success: Set '" << key << "' to '" << value << "'\n";
        return 0; // Exit so we don't accidentally launch a second engine instance
    }

    // Standard/Daemon Mode
    // Load config and launch engine
    ASCIIpaper::Engine::Config config;
    if (config.Load(configPath)) {
        ASCII_COUT << "Loaded config.ini successfully.\n";
    } else {
        ASCII_COUT << "No config.ini found. Using default engine settings.\n";
    }

    // Create window and attach to background
    ASCIIpaper::Engine::Window window("ASCIIpaper", 1920, 1080);
    if (!window.Initialize())
        return -1;

    ASCIIpaper::Platform::AttachToDesktop(window.GetNativeWindow());

    ASCIIpaper::Engine::Renderer renderer(window.GetNativeWindow());
    if (!renderer.Initialize())
        return -1;

    ASCIIpaper::Engine::CharacterGrid grid(120, 67);
    ASCIIpaper::Engine::SceneManager sceneManager;

    /*
     * Wrap the scene creation inside a lambda function.
     * This allows the engine to instantly wipe the current scene and load a brand
     * new one at runtime whenever the config is modified
     */
    auto loadScene = [&]() {
        // Load config variables
        std::string activeScene = config.GetString("scene", "aquarium");
        int fishCount = config.GetInt("fish_count", 6);
        int bubbleCount = config.GetInt("bubble_count", 15);
        int jellyCount = config.GetInt("jellyfish_count", 3);
        int shrimpCount = config.GetInt("shrimp_count", 12);
        int carCount = config.GetInt("car_count", 12);
        int starCount = config.GetInt("star_count", 40);
        std::string weatherMode = config.GetString("weather", "rain");
        bool systemSync = config.GetBool("system_sync", false);

        if (activeScene == "random") {
            std::mt19937 rng(std::random_device{}());
            std::uniform_int_distribution<int> dist(0, 1);
            activeScene = (dist(rng) == 0) ? "aquarium" : "city";
        }

        // Load selected scene
        if (activeScene == "city") {
            sceneManager.ChangeScene(std::make_unique<ASCIIpaper::Worlds::CityScene>(
                120, 67, carCount, starCount, weatherMode, systemSync));
        } else {
            sceneManager.ChangeScene(std::make_unique<ASCIIpaper::Worlds::AquariumScene>(
                120, 67, fishCount, bubbleCount, jellyCount, shrimpCount, systemSync));
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