#pragma once

#include <vector>
#include <random>

#include "engine/scene.h"
#include "engine/types.h"

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
    };

    // Twinkling star
    struct Star {
        int x, y;
        float timer;
        float threshold; // How long to wait before twinkling
        bool isTwinkling;
    };

    // Occasional UFO
    enum class UfoState {
        Waiting,
        Entering,
        Hovering,
        Leaving
    };

    struct Ufo {
        float x, y;
        UfoState state;
        float timer;
        float spawnThreshold; // Target time to wait before spawning
    };

    class CityScene : public Engine::Scene {
    public:
        CityScene(int width, int height, int carCount, int starCount);

        void Update(float deltaTime) override;
        void Draw(Engine::CharacterGrid& grid) override;

    private:
        int m_width;
        int m_height;
        float m_timeAccumulator;

        // Car
        int m_carCount;

        // Star
        int m_starCount;

        // Train variables
        float m_trainTimer;
        float m_trainX;
        bool m_trainActive;

        // UFO
        Ufo m_ufo;

        std::vector<Building> m_buildings;
        std::vector<Car> m_cars;
        std::vector<Star> m_stars;
        
        std::mt19937 m_rng;

        void InitializeWorld();
    };

} // namespace ASCIIpaper::Worlds