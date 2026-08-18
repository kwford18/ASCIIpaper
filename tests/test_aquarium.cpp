#include <gtest/gtest.h>
#include "worlds/aquarium.h"

using namespace ASCIIpaper::Worlds;

TEST(AquariumTest, HonorsConfigurationLimits) {
    int expectedFish = 15;
    int expectedBubbles = 5;
    int expectedJellies = 2;
    
    // Create an Aquarium instance
    AquariumScene scene(100, 50, expectedFish, expectedBubbles, expectedJellies, false);
    
    // Assuming you have getters like GetFishCount() in your scene:
    // EXPECT_EQ(scene.GetFishCount(), expectedFish);
}

TEST(AquariumTest, EntitiesUpdateOverTime) {
    AquariumScene scene(100, 50, 1, 0, 0, false);
    
    // Force the simulation forward by 1 second
    scene.Update(1.0f);
    
    // You should test that the grid has changed, or that the entity coordinates 
    // are different than they were at time 0.0f.
}