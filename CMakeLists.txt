cmake_minimum_required(VERSION 3.20)
project(ASCIIpaper VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ========== DEVELOPMENT AND TIDYING OPTIONS ==========
option(ASCII_DEBUG_MODE "Enable debugging tools, logging, and tests" OFF)

# Windows Global API Definitions (Required for both Debug and Release)
if(WIN32)
    add_compile_definitions(
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        _WIN32_WINNT=0x0601
        WINVER=0x0601
        _WIN32_IE=0x0600
    )
endif()


# ========== ONLY ENABLE DEV TOOLS IF IN DEBUG MODE ==========
if(ASCII_DEBUG_MODE)
    add_compile_definitions(ASCII_DEBUG_MODE)

    # Stricter compiler warnings for development
    if(MSVC)
        add_compile_options(/W4 /EHsc) # MSVC strict warnings
    else()
        add_compile_options(-Wall -Wextra -Wpedantic) # Clang strict warnings
    endif()

    # Memory Profiling with AddressSanitizer 
    set(USE_ASAN OFF) # Turned off by default to avoid linker clashes during general development

    if(USE_ASAN)
        if(MSVC)
            add_compile_options(/fsanitize=address)
            add_link_options(/INCREMENTAL:NO) # Prevents LNK4300 warning
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

    # Disable clang-tidy for GoogleTest and GoogleMock to avoid unnecessary warnings
    if(TARGET gtest)
        set_target_properties(gtest gtest_main gmock gmock_main PROPERTIES CXX_CLANG_TIDY "")
    endif()
endif()

# ========== Fetch SDL3 ==========
message(STATUS "Fetching SDL3 from source...")
include(FetchContent) # Re-include here in case Debug Mode is OFF and GTest didn't load it
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        main # Pulls the latest version
)
# Disable SDL3's internal tests and examples to speed up compile time
set(SDL_TEST_LIBRARY OFF CACHE BOOL "Disable SDL3 test library" FORCE)
set(SDL_TESTS OFF CACHE BOOL "Disable SDL3 tests" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "Disable SDL3 examples" FORCE)

# Disable unused SDL3 subsystems that are unneeded
set(SDL_AUDIO OFF CACHE BOOL "Disable SDL Audio" FORCE)
set(SDL_GPU OFF CACHE BOOL "Disable SDL GPU" FORCE)
set(SDL_CAMERA OFF CACHE BOOL "Disable SDL Camera" FORCE)
set(SDL_JOYSTICK OFF CACHE BOOL "Disable SDL Joystick" FORCE)
set(SDL_HAPTIC OFF CACHE BOOL "Disable SDL Haptic" FORCE)
set(SDL_HIDAPI OFF CACHE BOOL "Disable SDL HIDAPI" FORCE)
set(SDL_POWER OFF CACHE BOOL "Disable SDL Power" FORCE)
set(SDL_SENSOR OFF CACHE BOOL "Disable SDL Sensor" FORCE)
set(SDL_DIALOG OFF CACHE BOOL "Disable SDL Dialog" FORCE)
set(SDL_TRAY OFF CACHE BOOL "Disable SDL Tray" FORCE)
set(SDL_NOTIFICATION OFF CACHE BOOL "Disable SDL Notification" FORCE)

FetchContent_MakeAvailable(SDL3)

# Disable clang-tidy for SDL3 to avoid unnecessary warnings safely
if(TARGET SDL3-shared)
    set_target_properties(SDL3-shared PROPERTIES CXX_CLANG_TIDY "")
endif()
if(TARGET SDL3-static)
    set_target_properties(SDL3-static PROPERTIES CXX_CLANG_TIDY "")
endif()

# ========== Core Engine Target ==========
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

# Platform Backend Target
add_library(Platform STATIC platform/desktop.h)
target_include_directories(Platform PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Platform PRIVATE SDL3::SDL3)

# Selectively compile the correct desktop integration
if(WIN32)
    target_sources(Platform PRIVATE platform/windows_desktop.cpp)
elseif(UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
    target_sources(Platform PRIVATE platform/linux_desktop.cpp)
    target_link_libraries(Platform PRIVATE X11::X11)
endif()

# Application Executable
add_executable(ASCIIpaper WIN32 app/main.cpp)
target_link_libraries(ASCIIpaper PRIVATE Engine Worlds Platform)

# Windows DLL Copy for ASCIIpaper (Must happen in Release too)
if(WIN32)
    add_custom_command(TARGET ASCIIpaper POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:SDL3::SDL3>
        $<TARGET_FILE_DIR:ASCIIpaper>
    )
endif()

# ========== ONLY BUILD TESTS IF IN DEBUG MODE ==========
if(ASCII_DEBUG_MODE)
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

    # Windows DLL Copy for Tests
    if(WIN32)
        add_custom_command(TARGET EngineTests POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3>
            $<TARGET_FILE_DIR:EngineTests>
        )
    endif()

    # Register the test with CTest (Delayed until execution to prevent Missing DLL crashes)
    include(GoogleTest)
    gtest_discover_tests(EngineTests DISCOVERY_MODE PRE_TEST)
endif()