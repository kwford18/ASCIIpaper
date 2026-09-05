#include "engine/scene_manager.h"
#include "worlds/aquarium.h"
#include "worlds/city.h"
#include <gtest/gtest.h>

using namespace ASCIIpaper::Engine;
using namespace ASCIIpaper::Worlds;

class SceneManagerTest : public ::testing::Test {
  protected:
    SceneManager manager;
};

TEST_F(SceneManagerTest, InitializationIsEmpty) {
    EXPECT_EQ(manager.GetCurrentScene(), nullptr);
}

TEST_F(SceneManagerTest, CanLoadAndSwapScenes) {
    // Load Aquarium
    manager.ChangeScene(std::make_unique<AquariumScene>(100, 50, 10, 5, 2, 12, false));
    EXPECT_NE(manager.GetCurrentScene(), nullptr);

    // Hot swap to City
    manager.ChangeScene(std::make_unique<CityScene>(100, 50, 10, 20, "rain", false));
    EXPECT_NE(manager.GetCurrentScene(), nullptr);
}