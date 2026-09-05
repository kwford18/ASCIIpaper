#include "engine/grid.h"
#include "worlds/aquarium.h"
#include <gtest/gtest.h>

using namespace ASCIIpaper::Worlds;
using namespace ASCIIpaper::Engine;

TEST(AquariumTest, HonorsConfigurationLimits) {
    int expectedFish = 15;
    int expectedBubbles = 5;
    int expectedJellies = 2;
    int expectedShrimps = 3;

    // Create an Aquarium instance
    AquariumScene scene(100, 50, expectedFish, expectedBubbles, expectedJellies, expectedShrimps,
                        false);

    // The scene should strictly honor the configuration sizes
    EXPECT_EQ(scene.GetFishCount(), expectedFish);
    EXPECT_EQ(scene.GetBubbleCount(), expectedBubbles);
    EXPECT_EQ(scene.GetJellyfishCount(), expectedJellies);
    EXPECT_EQ(scene.GetShrimpCount(), expectedShrimps);
}

TEST(AquariumTest, EntitiesUpdateOverTime) {
    // Spin up an aquarium with just 1 moving fish
    AquariumScene scene(100, 50, 1, 0, 0, 0, false);
    CharacterGrid gridBefore(100, 50);
    CharacterGrid gridAfter(100, 50);

    // Capture the static initial frame
    scene.Draw(gridBefore);

    // Force the simulation forward by 1 second
    scene.Update(1.0f);

    // Capture the new frame
    scene.Draw(gridAfter);

    // Prove the fish actually moved across the grid
    bool hasDifferences = false;
    for (int y = 0; y < 50; ++y) {
        for (int x = 0; x < 100; ++x) {
            if (gridBefore.GetCell(x, y).character != gridAfter.GetCell(x, y).character) {
                hasDifferences = true;
                break;
            }
        }
    }

    EXPECT_TRUE(hasDifferences) << "Entities did not move on the grid after Update() was called.";
}