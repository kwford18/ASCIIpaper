#include "engine/Grid.h"

namespace Aquarium::Engine {

    CharacterGrid::CharacterGrid(int width, int height) 
        : m_width(width), m_height(height), m_cells(width * height) {
    }

    void CharacterGrid::SetCell(int x, int y, char c) {
        // Fallback to our classic green if no color is provided
        SetCell(x, y, c, 100, 255, 100); 
    }

    void CharacterGrid::SetCell(int x, int y, char c, uint8_t r, uint8_t g, uint8_t b) {
        if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
            int index = y * m_width + x;
            m_cells[index].character = c;
            m_cells[index].r = r;
            m_cells[index].g = g;
            m_cells[index].b = b;
        }
    }

    Cell CharacterGrid::GetCell(int x, int y) const {
        if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
            return m_cells[y * m_width + x];
        }
        return Cell{}; // Return empty cell if out of bounds
    }

    int CharacterGrid::GetWidth() const { return m_width; }
    int CharacterGrid::GetHeight() const { return m_height; }

    void CharacterGrid::Clear() {
        for (auto& cell : m_cells) {
            cell.character = ' ';
        }
    }

} // namespace Aquarium::Engine