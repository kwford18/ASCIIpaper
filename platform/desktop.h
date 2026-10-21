#pragma once
#include <SDL3/SDL.h>

namespace ASCIIpaper::Platform {

// Detaches the window from standard OS management
// and forces it to draw as the desktop background.
void AttachToDesktop(SDL_Window* window);
bool ShouldQuit();
void ShutdownDesktopIntegration(SDL_Window* window);

// For hot config reloads
bool HasConfigChanged();
void ClearConfigChanged();

} // namespace ASCIIpaper::Platform