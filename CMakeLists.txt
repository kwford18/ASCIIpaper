cmake_minimum_required(VERSION 3.20)
project(ASCIIpaper VERSION 0.1.0 LANGUAGES CXX)

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
    GIT_TAG        main # Pulls the latest version
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
    engine/renderer.cpp
    engine/timer.cpp
    engine/config.cpp
    engine/scene_manager.cpp
)
target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Engine PUBLIC SDL3::SDL3)

# Simulation Worlds Target
add_library(Worlds STATIC
    worlds/aquarium.cpp
    worlds/city.cpp
)
target_include_directories(Worlds PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Worlds PRIVATE Engine)

# Platform Backend Target
add_library(Platform STATIC)
target_include_directories(Platform PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Platform PRIVATE SDL3::SDL3)

# Only compile the Windows code if we are building on Windows
if(WIN32)
    target_sources(Platform PRIVATE platform/windows_desktop.cpp)
endif()

# Application Executable
# The WIN32 tells Windows to hide the console window
add_executable(ASCIIpaper WIN32 app/main.cpp)
target_link_libraries(ASCIIpaper PRIVATE Engine Worlds Platform)

# Unit Tests
add_executable(EngineTests 
    tests/test_grid.cpp
    tests/test_timer.cpp
    tests/test_scene.cpp
    tests/test_config.cpp
)

target_link_libraries(EngineTests PRIVATE Engine Worlds GTest::gtest_main)

# Register the test with CTest
include(GoogleTest)

# Windows DLL Copy
# Ensure the SDL3.dll is copied right next to the executables so they can run
if(WIN32)
    # Copy for the main application
    add_custom_command(TARGET ASCIIpaper POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3::SDL3>
        $<TARGET_FILE_DIR:ASCIIpaper>
    )
    
    # Copy for the test executable to fix the 0xc0000135 crash
    add_custom_command(TARGET EngineTests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3::SDL3>
        $<TARGET_FILE_DIR:EngineTests>
    )
endif()

# Ensure config.ini is copied to the executable directory so it can be read at runtime
add_custom_command(TARGET ASCIIpaper POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/config.ini"
    $<TARGET_FILE_DIR:ASCIIpaper>
)

gtest_discover_tests(EngineTests)