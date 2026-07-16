#include "worlds/aquarium.h"

namespace Aquarium::Worlds {

    AquariumScene::AquariumScene(int width, int height) 
        : m_width(width), m_height(height), m_timeAccumulator(0.0f) {
    }

    void AquariumScene::Update(float deltaTime) {
        m_timeAccumulator += deltaTime;
        // Coming next: Update fish and bubble positions
    }

    void AquariumScene::Draw(Engine::CharacterGrid& grid) {
        // Wipe the canvas clean every frame
        grid.Clear();
        
        // Draw the boundaries
        for (int x = 0; x < m_width; ++x) {
            grid.SetCell(x, 0, '#');
            grid.SetCell(x, m_height - 1, '#');
        }
        for (int y = 0; y < m_height; ++y) {
            grid.SetCell(0, y, '#');
            grid.SetCell(m_width - 1, y, '#');
        }

        // Draw a static fish for testing
        grid.SetCell(23, 18, '>');
        grid.SetCell(24, 18, '<');
        grid.SetCell(25, 18, '>');
        grid.SetCell(26, 18, 'O');
    }

} // namespace Aquarium::Worlds