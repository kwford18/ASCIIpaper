#include "engine/grid.h"
#include "worlds/city.h"
#include <gtest/gtest.h>

using namespace ASCIIpaper::Worlds;
using namespace ASCIIpaper::Engine;

TEST(CityTest, InitializationDoesNotCrash) {
    int expectedCars = 10;
    int expectedStars = 35;
    std::string expectedWeather = "snow";

    // Create a City instance with standard config values
    CityScene scene(100, 50, expectedCars, expectedStars, expectedWeather, false);

    // Verify the entities populated the vectors correctly
    EXPECT_EQ(scene.GetCarCount(), expectedCars);
    EXPECT_EQ(scene.GetStarCount(), expectedStars);
}

TEST(CityTest, EntitiesUpdateOverTime) {
    // Spin up a tiny city with 1 car, no stars, and clear weather
    CityScene scene(100, 50, 1, 0, "clear", false);
    CharacterGrid gridBefore(100, 50);
    CharacterGrid gridAfter(100, 50);

    // Capture the static initial frame
    scene.Draw(gridBefore);

    // Force the simulation forward by 1 second
    scene.Update(1.0f);

    // Capture the new frame
    scene.Draw(gridAfter);

    // Prove the car actually drove across the grid
    bool hasDifferences = false;
    for (int y = 0; y < 50; ++y) {
        for (int x = 0; x < 100; ++x) {
            if (gridBefore.GetCell(x, y).character != gridAfter.GetCell(x, y).character) {
                hasDifferences = true;
                break;
            }
        }
    }

    EXPECT_TRUE(hasDifferences) << "Traffic did not move on the grid after Update() was called.";
}

TEST(CityTest, InvalidWeatherDefaultsSafely) {
    // If a user types a typo in the config or CLI (like "snnow")
    CityScene scene(100, 50, 10, 35, "snnow", false);

    // The scene should still build successfully and default to "none/clear"
    // rather than crashing the engine!
    SUCCEED();
}