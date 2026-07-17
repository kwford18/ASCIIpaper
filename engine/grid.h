#pragma once
#include <vector>

namespace Aquarium::Engine {

    struct Cell {
        char character = ' ';
        // Default to the original terminal green
        uint8_t r = 100;
        uint8_t g = 255;
        uint8_t b = 100;
    };

    class CharacterGrid {
    public:
        CharacterGrid(int width, int height);

        // Backward compatibility
        void SetCell(int x, int y, char c);

        // New SetCell for color
        void SetCell(int x, int y, char c, uint8_t r, uint8_t g, uint8_t b);

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