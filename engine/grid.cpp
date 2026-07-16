#include "engine/Grid.h"

namespace Aquarium::Engine {

    CharacterGrid::CharacterGrid(int width, int height) 
        : m_width(width), m_height(height), m_cells(width * height) {
    }

    void CharacterGrid::SetCell(int x, int y, char c) {
        if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
            m_cells[y * m_width + x].character = c;
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