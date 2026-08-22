#include "engine/grid.h"
#include "engine/weather.h"
#include <gtest/gtest.h>

TEST(WeatherSystemTest, InitializesAndDrawsParticles) {
    ASCIIpaper::Engine::WeatherSystem weather;
    ASCIIpaper::Engine::CharacterGrid grid(100, 100);

    // Initialize rain
    weather.Initialize(ASCIIpaper::Engine::WeatherType::Rain, 100, 100);
    weather.Draw(grid);

    // Count how many particles were drawn
    int particleCount = 0;
    for (int y = 0; y < 100; ++y) {
        for (int x = 0; x < 100; ++x) {
            if (grid.GetCell(x, y).character != ' ') {
                particleCount++;
            }
        }
    }

    // We expect rain to spawn 150 particles
    EXPECT_GT(particleCount, 0);
    EXPECT_LE(particleCount, 150);
}