#include <gtest/gtest.h>
#include "worlds/aquarium.h"
#include "engine/grid.h"

using namespace ASCIIpaper::Worlds;
using namespace ASCIIpaper::Engine;

TEST(AquariumSceneTest, Initialization) {
    // Pass default entity counts (5 fish, 12 bubbles, 3 jellyfish)
    ASSERT_NO_THROW(AquariumScene scene(50, 37, 5, 12, 3));
}

TEST(AquariumSceneTest, DrawPopulatesGridBoundaries) {
    // Use a smaller 10x10 grid/scene for easier boundary testing
    CharacterGrid grid(10, 10);
    AquariumScene scene(10, 10, 0, 0, 0);

    // Clear randomly spawned entities so they don't corrupt static boundary test
    scene.ClearEntities();
    scene.Draw(grid);
    
    EXPECT_EQ(grid.GetCell(0, 0).character, ' ');
    EXPECT_EQ(grid.GetCell(9, 9).character, ' ');
    EXPECT_EQ(grid.GetCell(5, 5).character, ' ');
}

TEST(AquariumSceneTest, DrawClearsPreviousFrame) {
    CharacterGrid grid(10, 10);
    AquariumScene scene(10, 10, 0, 0, 0);

    // Clear randomly spawned entities so they don't corrupt test
    scene.ClearEntities();

    // Manually put "garbage" in the grid
    grid.SetCell(5, 5, 'X');

    // Drawing the scene should wipe the 'X' and replace it with water ' '
    scene.Draw(grid);

    EXPECT_EQ(grid.GetCell(5, 5).character, ' ');
}

TEST(AquariumSceneTest, SimulationSpawnsEntities) {
    AquariumScene scene(50, 37, 5, 12, 3);
    
    // The scene should automatically seed life when initialized
    EXPECT_GT(scene.GetFishCount(), 0);
    EXPECT_GT(scene.GetBubbleCount(), 0);
}

TEST(AquariumSceneTest, UpdateMovesEntities) {
    CharacterGrid grid_before(50, 37);
    CharacterGrid grid_after(50, 37);
    AquariumScene scene(50, 37, 5, 12, 3);

    // Capture the static initial frame
    scene.Draw(grid_before);
    
    // Simulate 1 full second of simulation time passing
    scene.Update(1.0f);
    
    // Capture the new frame
    scene.Draw(grid_after);

    // Ensure the simulation actually changed the visual output. 
    // If the fish/bubbles moved, the grids should no longer be identical.
    bool hasDifferences = false;
    for(int y = 1; y < 36; ++y) {
        for(int x = 1; x < 49; ++x) {
            if (grid_before.GetCell(x, y).character != grid_after.GetCell(x, y).character) {
                hasDifferences = true;
                break;
            }
        }
    }
    
    EXPECT_TRUE(hasDifferences) << "The visual grid did not change after calling Update(1.0f). Entities are not moving!";
}