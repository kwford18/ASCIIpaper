#include "worlds/aquarium.h"

namespace Aquarium::Worlds {

    AquariumScene::AquariumScene(int width, int height) 
        : m_width(width), m_height(height), m_timeAccumulator(0.0f), m_rng(std::random_device{}()) {
        InitializeWorld();
    }

    void AquariumScene::ClearEntities() {
        m_fishes.clear();
        m_bubbles.clear();
    }

    void AquariumScene::InitializeWorld() {
        // Setup random distributions to keep entities inside the boundaries
        std::uniform_real_distribution<float> xDist(2.0f, m_width - 5.0f);
        std::uniform_real_distribution<float> yDist(2.0f, m_height - 3.0f);
        std::uniform_real_distribution<float> fSpeedDist(2.0f, 6.0f);
        std::uniform_int_distribution<int> dirDist(0, 1);

        // Randomly spawn 5 fish
        for (int i = 0; i < 5; ++i) {
            m_fishes.push_back({
                xDist(m_rng), 
                yDist(m_rng), 
                fSpeedDist(m_rng), 
                dirDist(m_rng) == 0 ? Direction::Left : Direction::Right
            });
        }

        std::uniform_real_distribution<float> bSpeedDist(1.0f, 3.0f);
        std::uniform_int_distribution<int> sizeDist(0, 1);

        // Randomly spawn 12 bubbles
        for (int i = 0; i < 12; ++i) {
            m_bubbles.push_back({
                xDist(m_rng), yDist(m_rng), bSpeedDist(m_rng), sizeDist(m_rng) == 0 ? 'o' : 'O'
            });
        }
    }

    void AquariumScene::Update(float deltaTime) {
        m_timeAccumulator += deltaTime;

        // Move Fish
        for (auto& fish : m_fishes) {
            fish.x += fish.speed * static_cast<int>(fish.direction) * deltaTime;
            
            // Wall collisions (fish is 3 characters long)
            if (fish.x <= 1.0f) {
                fish.x = 1.0f;
                fish.direction = Direction::Right;
            } else if (fish.x >= m_width - 4.0f) {
                fish.x = m_width - 4.0f;
                fish.direction = Direction::Left;
            }
        }

        // Move Bubbles
        for (auto& bubble : m_bubbles) {
            bubble.y -= bubble.speed * deltaTime;

            // If a bubble hits the top, respawn it at the bottom with a new random x position
            if (bubble.y <= 1.0f) {
                bubble.y = static_cast<float>(m_height - 2);
                std::uniform_real_distribution<float> xDist(1.0f, m_width - 2.0f);
                bubble.x = xDist(m_rng);
            }
        }
    }

    void AquariumScene::Draw(Engine::CharacterGrid& grid) {
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

        // Draw Bubbles
        for (const auto& bubble : m_bubbles) {
            int ix = static_cast<int>(bubble.x);
            int iy = static_cast<int>(bubble.y);
            grid.SetCell(ix, iy, bubble.symbol);
        }

        // Draw Fish
        for (const auto& fish : m_fishes) {
            int ix = static_cast<int>(fish.x);
            int iy = static_cast<int>(fish.y);
            
            // Safety bounds check
            if (ix > 0 && ix < m_width - 3 && iy > 0 && iy < m_height - 1) {
                if (fish.direction == Direction::Right) {   // Facing Right
                    grid.SetCell(ix, iy, '>');
                    grid.SetCell(ix + 1, iy, '<');
                    grid.SetCell(ix + 2, iy, '>');
                } else {                                    // Facing Left
                    grid.SetCell(ix, iy, '<');
                    grid.SetCell(ix + 1, iy, '>');
                    grid.SetCell(ix + 2, iy, '<');
                }
            }
        }
    }

} // namespace Aquarium::Worlds