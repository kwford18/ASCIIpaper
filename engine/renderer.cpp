#include "engine/renderer.h"
#include "engine/logger.h"

namespace ASCIIpaper::Engine {

Renderer::Renderer(SDL_Window* window) : m_window(window), m_renderer(nullptr) {
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
        ASCII_CERR << "SDL_CreateRenderer failed: " << SDL_GetError() << '\n';
        return false;
    }
    return true;
}

void Renderer::DrawGrid(const CharacterGrid& grid) {
    // Clear the screen to invisible
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 0);
    SDL_RenderClear(m_renderer);

    SDL_SetRenderScale(m_renderer, 2.0f, 2.0f);

    const int cellWidth = 8;
    const int cellHeight = 8;

    for (int y = 0; y < grid.GetHeight(); ++y) {
        for (int x = 0; x < grid.GetWidth(); ++x) {

            // Grab the entire Cell struct instead of just the character
            auto cell = grid.GetCell(x, y);

            if (cell.character != ' ') {
                // Apply the cell's specific RGB color before drawing
                SDL_SetRenderDrawColor(m_renderer, cell.r, cell.g, cell.b, 255);

                char str[2] = {cell.character, '\0'};
                SDL_RenderDebugText(m_renderer, static_cast<float>(x * cellWidth),
                                    static_cast<float>(y * cellHeight), str);
            }
        }
    }

    SDL_SetRenderScale(m_renderer, 1.0f, 1.0f);
    SDL_RenderPresent(m_renderer);
}

} // namespace ASCIIpaper::Engine