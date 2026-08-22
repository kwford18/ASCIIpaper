#include "engine/timer.h"
#include <SDL3/SDL.h>
#include <gtest/gtest.h>

using namespace ASCIIpaper::Engine;

TEST(TimerTest, Initialization) {
    // Initialize core SDL subsystems (no video/window required)
    ASSERT_TRUE(SDL_Init(0)) << "SDL must initialize for Timer tests.";

    Timer timer(60);

    // Delta time should be zero before the first Tick()
    EXPECT_EQ(timer.GetDeltaTime(), 0.0f);

    SDL_Quit();
}