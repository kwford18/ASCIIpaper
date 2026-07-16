#include "engine/renderer.h"
#include <iostream>

namespace Aquarium::Engine {

    Renderer::Renderer(SDL_Window* window) 
        : m_window(window), m_renderer(nullptr) {
    }

    Renderer::~Renderer() {
        if (m_renderer) {
            SDL_DestroyRenderer(m_renderer);
        }
    }

    bool Renderer::Initialize() {
        // Create the 2D rendering context
        m_renderer = SDL_CreateRenderer(m_window, nullptr);
        if (!m_renderer) {
            std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
            return false;
        }
        return true;
    }

    void Renderer::DrawGrid(const CharacterGrid& grid) {
        SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
        SDL_RenderClear(m_renderer);

        // Set text color to a soft, terminal green
        SDL_SetRenderDrawColor(m_renderer, 100, 255, 100, 255);

        // SDL_RenderDebugText uses an 8x8 pixel font
        // Scale the renderer by 2.0 so the grid cells are 16x16 pixels.
        SDL_SetRenderScale(m_renderer, 2.0f, 2.0f);
        
        const int cellWidth = 8;
        const int cellHeight = 8;

        // Iterate through the grid and draw the characters
        for (int y = 0; y < grid.GetHeight(); ++y) {
            for (int x = 0; x < grid.GetWidth(); ++x) {
                char c = grid.GetCell(x, y).character;
                
                // Only draw non-empty characters to save rendering time
                if (c != ' ') {
                    // SDL_RenderDebugText expects a null-terminated string
                    char str[2] = {c, '\0'};
                    SDL_RenderDebugText(m_renderer, x * cellWidth, y * cellHeight, str);
                }
            }
        }

        // Reset the scale for the next frame
        SDL_SetRenderScale(m_renderer, 1.0f, 1.0f);

        // Swap the buffers to present the drawn frame to the screen
        SDL_RenderPresent(m_renderer);
    }

} // namespace Aquarium::Engine