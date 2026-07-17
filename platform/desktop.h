#pragma once
#include <SDL3/SDL.h>

namespace Aquarium::Platform {

    // Detaches the window from standard OS management 
    // and forces it to draw as the desktop background.
    void AttachToDesktop(SDL_Window* window);
    bool ShouldQuit();
    void ShutdownDesktopIntegration();

} // namespace Aquarium::Platform