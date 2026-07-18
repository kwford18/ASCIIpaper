#include "worlds/city.h"
#include <cmath>

namespace Aquarium::Worlds {

    CityScene::CityScene(int width, int height) 
        : m_width(width), m_height(height), m_timeAccumulator(0.0f) {
    }

    void CityScene::Update(float deltaTime) {
        m_timeAccumulator += deltaTime;
    }

    void CityScene::Draw(Engine::CharacterGrid& grid) {
        grid.Clear();

        // Draw a simple static building structure
        for (int y = m_height - 20; y < m_height; ++y) {
            for (int x = 10; x < 30; ++x) {
                // Gray-ish building blocks
                grid.SetCell(x, y, '#', 120, 120, 130);
            }
        }

        // Draw a bobbing moon in the sky
        int moonY = static_cast<int>(10.0f + std::sin(m_timeAccumulator) * 3.0f);
        grid.SetCell(40, moonY, 'C', 255, 255, 150);
    }

} // namespace Aquarium::Worlds