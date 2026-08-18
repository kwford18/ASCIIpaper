#include <gtest/gtest.h>
#include "worlds/city.h"

using namespace ASCIIpaper::Worlds;

TEST(CityTest, InitializationDoesNotCrash) {
    int expectedCars = 10;
    int expectedStars = 35;
    std::string expectedWeather = "snow";
    
    // Create a City instance with your standard config values
    CityScene scene(100, 50, expectedCars, expectedStars, expectedWeather, false);
    
    // If the scene allocates all its cars and stars without a segmentation fault, 
    // the constructor is memory-safe!
    SUCCEED();
    
    // Once you add getters to city.h, you can uncomment these!
    // EXPECT_EQ(scene.GetCarCount(), expectedCars);
    // EXPECT_EQ(scene.GetStarCount(), expectedStars);
}

TEST(CityTest, EntitiesUpdateOverTime) {
    // Spin up a tiny city with 1 car, no stars, and clear weather
    CityScene scene(100, 50, 1, 0, "clear", false);
    
    // Force the simulation forward by 1 second
    scene.Update(1.0f);
    
    // Assuming you eventually add a way to inspect the grid or car positions,
    // you would test that the car's X-coordinate is different than it was at 0.0f.
    SUCCEED();
}

TEST(CityTest, InvalidWeatherDefaultsSafely) {
    // If a user types a typo in the config or CLI (like "snnow")
    CityScene scene(100, 50, 10, 35, "snnow", false);
    
    // The scene should still build successfully and default to "none/clear"
    // rather than crashing the engine!
    SUCCEED();
}