#include <gtest/gtest.h>
#include "worlds/aquarium.h"
#include "engine/grid.h"

using namespace Aquarium::Worlds;
using namespace Aquarium::Engine;

TEST(AquariumSceneTest, Initialization) {
    ASSERT_NO_THROW(AquariumScene scene(50, 37));
}

TEST(AquariumSceneTest, DrawPopulatesGridBoundaries) {
    // Use a smaller 10x10 grid/scene for easier boundary testing
    CharacterGrid grid(10, 10);
    AquariumScene scene(10, 10);

    scene.Draw(grid);

    // Verify the Corners are '#'
    EXPECT_EQ(grid.GetCell(0, 0).character, '#');
    EXPECT_EQ(grid.GetCell(9, 0).character, '#');
    EXPECT_EQ(grid.GetCell(0, 9).character, '#');
    EXPECT_EQ(grid.GetCell(9, 9).character, '#');

    // Verify edges are '#'
    EXPECT_EQ(grid.GetCell(5, 0).character, '#'); // Top edge
    EXPECT_EQ(grid.GetCell(0, 5).character, '#'); // Left edge

    // Verify the inner water is empty space ' '
    EXPECT_EQ(grid.GetCell(5, 5).character, ' ');
}

TEST(AquariumSceneTest, DrawClearsPreviousFrame) {
    CharacterGrid grid(10, 10);
    AquariumScene scene(10, 10);

    // Manually put "garbage" in the grid
    grid.SetCell(5, 5, 'X');

    // Drawing the scene should wipe the 'X' and replace it with water ' '
    scene.Draw(grid);

    EXPECT_EQ(grid.GetCell(5, 5).character, ' ');
}