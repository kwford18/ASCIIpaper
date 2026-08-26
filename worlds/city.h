#pragma once

#include <cstdint>
#include <random>
#include <vector>

#include "engine/scene.h"
#include "engine/system_monitor.h"
#include "engine/types.h"
#include "engine/weather.h"

namespace ASCIIpaper::Worlds {

// A single skyscraper
struct Building {
    int x;
    int width;
    int height;
    uint8_t r, g, b;
    std::vector<bool> windows; // Keeps track of which windows are lit up
};

// A vehicle on the highway
struct Car {
    float x;
    int y;
    float speed;
    Engine::Direction direction;
    uint8_t r, g, b;
    bool isLeaving = false; // Flag for highway exits
};

// Twinkling star
struct Star {
    int x, y;
    float timer;
    float threshold; // How long to wait before twinkling
    bool isTwinkling;
};

// Occasional UFO
enum class UfoState { Waiting, Entering, Hovering, Leaving };

struct Ufo {
    float x, y;
    UfoState state;
    float timer;
    float spawnThreshold; // Target time to wait before spawning
};

class CityScene : public Engine::Scene {
  public:
    CityScene(int width, int height, int carCount, int starCount, const std::string& weather,
              bool systemSync);

    void Update(float deltaTime) override;
    void Draw(Engine::CharacterGrid& grid) override;

#ifdef ASCII_DEBUG_MODE
    // Exposed so unit tests can verify spawning
    int GetCarCount() const {
        return static_cast<int>(m_cars.size());
    }
    int GetStarCount() const {
        return static_cast<int>(m_stars.size());
    }
#endif

  private:
    int m_width;
    int m_height;
    float m_timeAccumulator;

    // Car
    int m_baseCarCount; // Tracks baseline traffic

    // Star
    int m_starCount;

    // Train variables
    float m_trainTimer;
    float m_trainX;
    bool m_trainActive;
    float m_trainSpeed;
    int m_trainCars;

    // UFO
    Ufo m_ufo;

    // Weather
    std::string m_weatherStr;
    Engine::WeatherSystem m_weather;

    // Vectors
    std::vector<Building> m_buildings;
    std::vector<Car> m_cars;
    std::vector<Star> m_stars;

    // RNG
    std::mt19937 m_rng;

    // System Monitr
    bool m_systemSync;
    Engine::SystemMonitor m_sysMonitor;
    float m_lastRamUsage = -1.0f; // Track RAM to prevent flickering lights

    void InitializeWorld();

    // Modular update helpers
    void UpdateSystemMonitor(float deltaTime, float& cpuMultiplier);
    void UpdateTraffic(float deltaTime, float cpuMultiplier);
    void UpdateTrain(float deltaTime);
    void UpdateUfo(float deltaTime);
    void UpdateEnvironment(float deltaTime);

    // Modular draw helpers
    void DrawBackground(Engine::CharacterGrid& grid);
    void DrawCityscape(Engine::CharacterGrid& grid);
    void DrawTraffic(Engine::CharacterGrid& grid);
    void DrawForeground(Engine::CharacterGrid& grid);
    void DrawHUD(Engine::CharacterGrid& grid);
};

} // namespace ASCIIpaper::Worlds