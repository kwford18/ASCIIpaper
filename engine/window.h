#pragma once
#include <SDL3/SDL.h>
#include <string>

namespace ASCIIpaper::Engine {

class Window {
  public:
    Window(const std::string& title, int width, int height);
    ~Window();

    // Initializes the SDL3 video subsystem and creates the window
    bool Initialize();

    // Processes OS events (resizing, closing, etc.)
    void PollEvents();

    // Returns true if the user requested to close the window
    bool ShouldClose() const;

    // Getters
    int GetWidth() const;
    int GetHeight() const;
    SDL_Window* GetNativeWindow() const;

  private:
    std::string m_title;
    int m_width;
    int m_height;
    bool m_shouldClose;

    SDL_Window* m_window;
};

} // namespace ASCIIpaper::Engine