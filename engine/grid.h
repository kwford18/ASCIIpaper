#pragma once
#include <vector>

namespace Aquarium::Engine {

    struct Cell {
        char character = ' ';
        // Color support planned for Milestone 3/6 can be added here
    };

    class CharacterGrid {
    public:
        CharacterGrid(int width, int height);

        void SetCell(int x, int y, char c);
        Cell GetCell(int x, int y) const;
        
        int GetWidth() const;
        int GetHeight() const;
        void Clear();

    private:
        int m_width;
        int m_height;
        std::vector<Cell> m_cells;
    };

} // namespace Aquarium::Engine