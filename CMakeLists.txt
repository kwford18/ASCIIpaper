cmake_minimum_required(VERSION 3.20)
project(ASCIIpaper VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ========== DEVELOPMENT AND TIDYING OPTIONS ==========
# Stricter compiler warnings for development
if(MSVC)
    add_compile_options(/W4 /EHsc) # MSVC strict warnings
else()
    add_compile_options(-Wall -Wextra -Wpedantic) # Clang strict warnings
endif()

# Memory Profiling with AddressSanitizer 
# TODO: Remove for 1.0 release since this is just for development and debugging
set(USE_ASAN ON) 

if(USE_ASAN)
    if(MSVC)
        add_compile_options(/fsanitize=address)
    else()
        add_compile_options(-fsanitize=address -g)
        add_link_options(-fsanitize=address)
    endif()
endif()

# clang-tidy
unset(CLANG_TIDY_EXE CACHE)

find_program(CLANG_TIDY_EXE 
    NAMES "clang-tidy"
    HINTS "C:/Program Files/LLVM/bin" "C:/Program Files (x86)/LLVM/bin"
)

if(CLANG_TIDY_EXE)
    message(STATUS "SUCCESS: clang-tidy found at ${CLANG_TIDY_EXE}")
    
    set(CMAKE_CXX_CLANG_TIDY 
        "${CLANG_TIDY_EXE}" 
        "-checks=-*,bugprone-*,performance-*,-bugprone-easily-swappable-parameters"
        "--header-filter=${CMAKE_CURRENT_SOURCE_DIR}/.*"
        "--extra-arg-before=--driver-mode=cl"
        "--extra-arg=/EHsc"
    )
else()
    message(WARNING "clang-tidy not found on system PATH or in default LLVM directories!")
endif()

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
    engine/weather.cpp
    engine/types.h
    engine/system_monitor.h
)
target_include_directories(Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Engine PUBLIC SDL3::SDL3)

# Selectively compile the correct monitor implementation
if(WIN32)
    target_sources(Engine PRIVATE engine/windows_monitor.cpp)
elseif(UNIX AND NOT APPLE)
    target_sources(Engine PRIVATE engine/linux_monitor.cpp)
endif()

# Simulation Worlds Target
add_library(Worlds STATIC
    worlds/aquarium.cpp
    worlds/city.cpp
)
target_include_directories(Worlds PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Worlds PRIVATE Engine)

# Platform Backend Target (Providing the header prevents empty static library errors)
add_library(Platform STATIC platform/desktop.h)
target_include_directories(Platform PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Platform PRIVATE SDL3::SDL3)

# Selectively compile the correct desktop integration
# NOTE: Linux development is currently paused. It may be added later on
if(WIN32)
    target_sources(Platform PRIVATE platform/windows_desktop.cpp)
elseif(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    target_sources(Platform PRIVATE platform/linux_desktop.cpp)
    target_link_libraries(Platform PRIVATE X11::X11)
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
    tests/test_weather.cpp
    tests/test_config.cpp
    tests/test_scene_manager.cpp
    tests/test_aquarium.cpp
    tests/test_monitor.cpp
    tests/test_city.cpp
)
target_link_libraries(EngineTests PRIVATE Engine Worlds GTest::gtest_main)

# Windows DLL Copy
# Ensure the SDL3.dll is copied right next to the executables so they can run
if(WIN32)
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

# Register the test with CTest 
include(GoogleTest)
gtest_discover_tests(EngineTests)