#pragma once
#include <SDL3/SDL.h>
#include "engine/grid.h"

namespace Aquarium::Engine {

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

} // namespace Aquarium::Engine