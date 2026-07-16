cmake_minimum_required(VERSION 3.20)
# Updated project name to wall-aquarium
project(wall-aquarium VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable testing globally
enable_testing()

# Fetch GoogleTest cross-platform
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Fetch SDL3
message(STATUS "Fetching SDL3 from source...")

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        main # Pulls the latest version. You can lock this to a specific release tag later.
)

# Disable SDL3's internal tests and examples to speed up compile time
set(SDL_TEST_LIBRARY OFF CACHE BOOL "Disable SDL3 test library" FORCE)
set(SDL_TESTS OFF CACHE BOOL "Disable SDL3 tests" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "Disable SDL3 examples" FORCE)

FetchContent_MakeAvailable(SDL3)

# Core Engine Target
add_library(Engine STATIC 
    engine/grid.cpp
    engine/window.cpp
)

# Explicitly declare include directories so the test executable can find the headers
target_include_directories(Engine PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(Engine PRIVATE SDL3::SDL3)

# Application Executable
add_executable(wall-aquarium app/main.cpp)
target_link_libraries(wall-aquarium PRIVATE Engine)

# --- Unit Tests ---
add_executable(EngineTests 
    tests/test_grid.cpp
)
target_link_libraries(EngineTests PRIVATE Engine GTest::gtest_main)

# Register the test with CTest
include(GoogleTest)
gtest_discover_tests(EngineTests)