#include "engine/config.h"
#include <gtest/gtest.h>
#include <sstream>

using namespace ASCIIpaper::Engine;

TEST(ConfigTest, FallbackToDefaultValues) {
    Config config;
    // No data loaded, should return the defaults we pass in
    EXPECT_EQ(config.GetInt("target_fps", 60), 60);
    EXPECT_FLOAT_EQ(config.GetFloat("fish_speed", 2.5f), 2.5f);
    EXPECT_EQ(config.GetString("theme", "dark"), "dark");
}

TEST(ConfigTest, ParsesValidStream) {
    std::string mockFileContent = "# This is a comment\n"
                                  "target_fps = 30\n"
                                  "fish_count = 10 \n"
                                  "  water_color = blue  \n"; // Testing weird spacing

    std::istringstream stream(mockFileContent);
    Config config;

    EXPECT_TRUE(config.LoadFromStream(stream));

    EXPECT_EQ(config.GetInt("target_fps", 60), 30);
    EXPECT_EQ(config.GetInt("fish_count", 5), 10);
    EXPECT_EQ(config.GetString("water_color", "green"), "blue");

    // Missing key should still return default
    EXPECT_EQ(config.GetInt("jellyfish_count", 2), 2);
}

TEST(ConfigTest, HandlesMalformedDataGracefully) {
    std::string mockFileContent = "target_fps = NOT_A_NUMBER\n";
    std::istringstream stream(mockFileContent);
    Config config;

    config.LoadFromStream(stream);

    // Should catch the exception and return the default value
    EXPECT_EQ(config.GetInt("target_fps", 60), 60);
}