#pragma once
#include "engine/grid.h"
#include <SDL3/SDL.h>

namespace ASCIIpaper::Engine {

class Renderer {
  public:
    Renderer(SDL_Window* window);
    ~Renderer();

    // Creates the SDL3 Renderer context
    bool Initialize();

    // Takes a CharacterGrid and draws it to the screen
    void DrawGrid(const CharacterGrid& grid);

  private:
    SDL_Window* m_window;
    SDL_Renderer* m_renderer;
};

} // namespace ASCIIpaper::Engine