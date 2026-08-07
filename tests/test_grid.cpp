#include <gtest/gtest.h>
#include "engine/grid.h"

using namespace ASCIIpaper::Engine;

TEST(CharacterGridTest, Initialization) {
    CharacterGrid grid(80, 24);
    EXPECT_EQ(grid.GetWidth(), 80);
    EXPECT_EQ(grid.GetHeight(), 24);

    // Grid should be initialized with spaces
    EXPECT_EQ(grid.GetCell(0, 0).character, ' ');
    EXPECT_EQ(grid.GetCell(79, 23).character, ' ');
}

TEST(CharacterGridTest, SetAndGetCell) {
    CharacterGrid grid(10, 10);
    
    grid.SetCell(5, 5, 'X');
    EXPECT_EQ(grid.GetCell(5, 5).character, 'X');
}

TEST(CharacterGridTest, OutOfBoundsHandling) {
    CharacterGrid grid(10, 10);
    
    // Setting out of bounds should not crash
    ASSERT_NO_THROW(grid.SetCell(-1, 5, 'A'));
    ASSERT_NO_THROW(grid.SetCell(10, 5, 'B'));
    ASSERT_NO_THROW(grid.SetCell(5, -1, 'C'));
    ASSERT_NO_THROW(grid.SetCell(5, 10, 'D'));

    // Getting out of bounds should return a default empty cell
    EXPECT_EQ(grid.GetCell(-1, -1).character, ' ');
    EXPECT_EQ(grid.GetCell(10, 10).character, ' ');
}

TEST(CharacterGridTest, ClearGrid) {
    CharacterGrid grid(5, 5);
    
    grid.SetCell(2, 2, 'O');
    EXPECT_EQ(grid.GetCell(2, 2).character, 'O');

    grid.Clear();
    EXPECT_EQ(grid.GetCell(2, 2).character, ' ');
}