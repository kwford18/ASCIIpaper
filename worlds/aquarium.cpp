#include "worlds/aquarium.h"
#include <cmath>

namespace Aquarium::Worlds {

    AquariumScene::AquariumScene(int width, int height) 
        : m_width(width), m_height(height), m_timeAccumulator(0.0f), m_rng(std::random_device{}()) {
        InitializeWorld();
    }

    void AquariumScene::ClearEntities() {
        m_fishes.clear();
        m_bubbles.clear();
        m_seaweeds.clear();
        m_jellyfishes.clear();
    }

    void AquariumScene::InitializeWorld() {
        // Setup random distributions to keep entities inside the boundaries
        std::uniform_real_distribution<float> xDist(2.0f, m_width - 5.0f);
        std::uniform_real_distribution<float> yDist(4.0f, m_height - 6.0f); // Keep them away from extreme edges
        std::uniform_real_distribution<float> fSpeedDist(2.0f, 5.0f);
        std::uniform_real_distribution<float> phaseDist(0.0f, 6.28f); // 0 to 2*PI for offsets
        std::uniform_int_distribution<int> dirDist(0, 1);

        // For random wandering
        std::uniform_real_distribution<float> vyDist(-1.5f, 1.5f);
        std::uniform_real_distribution<float> timerDist(1.0f, 5.0f);

        // Spawn Fish with wobble offsets
        for (int i = 0; i < 5; ++i) {
            float vx = fSpeedDist(m_rng);
            Direction dir = dirDist(m_rng) == 0 ? Direction::Left : Direction::Right;
            if (dir == Direction::Left) vx = -vx;

            m_fishes.push_back({
                xDist(m_rng), 
                yDist(m_rng), 
                vx, 
                vyDist(m_rng), 
                dir,
                phaseDist(m_rng),
                timerDist(m_rng) // Randomize when they first change direction
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

        // Distribtions for seaweed placement and height
        std::uniform_int_distribution<int> swXDist(2, m_width - 3);
        std::uniform_int_distribution<int> swHeightDist(3, 8); 
        std::uniform_real_distribution<float> swOffsetDist(0.0f, 6.28f);

        // Plant 6 stalks of seaweed at the bottom of the tank
        for (int i = 0; i < 6; ++i) {
            m_seaweeds.push_back({
                swXDist(m_rng),
                swHeightDist(m_rng),
                swOffsetDist(m_rng)
            });
        }

        // Spawn 3 jellyfish with random positions, speeds, and pulse offsets
        std::uniform_real_distribution<float> jSpeedDist(0.5f, 1.5f);
        for (int i = 0; i < 3; ++i) {
            m_jellyfishes.push_back({
                xDist(m_rng), 
                yDist(m_rng), 
                jSpeedDist(m_rng), 
                phaseDist(m_rng),
                VerticalDirection::Up // Start by floating upwards
            });
        }
    }

    void AquariumScene::Update(float deltaTime) {
        m_timeAccumulator += deltaTime;

        // Move Fish
        for (auto& fish : m_fishes) {
            
            // Randomly wander up and down over time
            fish.changeTimer -= deltaTime;
            if (fish.changeTimer <= 0.0f) {
                std::uniform_real_distribution<float> vyDist(-1.5f, 1.5f);
                std::uniform_real_distribution<float> timerDist(2.0f, 6.0f);
                fish.vy = vyDist(m_rng);
                fish.changeTimer = timerDist(m_rng);
            }

            // Apply 2D Velocity
            fish.x += fish.vx * deltaTime;
            fish.y += fish.vy * deltaTime;
            
            // X-Axis Wall Bouncing
            if (fish.x <= 1.0f) {
                fish.x = 1.0f;
                fish.vx = std::abs(fish.vx); // Force right
                fish.direction = Direction::Right; 
            } else if (fish.x >= m_width - 4.0f) {
                fish.x = m_width - 4.0f;
                fish.vx = -std::abs(fish.vx); // Force left
                fish.direction = Direction::Left;  
            }

            // Y-Axis Wall Bouncing
            if (fish.y <= 1.0f) {
                fish.y = 1.0f;
                fish.vy = std::abs(fish.vy); // Force down
            } else if (fish.y >= m_height - 3.0f) {
                fish.y = m_height - 3.0f;
                fish.vy = -std::abs(fish.vy); // Force up
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

        // Move jellyfish
        for (auto& jelly : m_jellyfishes) {
            // Pulse logic
            float pulse = std::sin(m_timeAccumulator * 3.0f + jelly.pulseOffset);
            float pulseMultiplier = 1.0f + 0.8f * pulse;
            
            // Apply speed combined with their current vertical direction
            jelly.y += jelly.speed * pulseMultiplier * static_cast<float>(jelly.verticalDir) * deltaTime;
            
            // Gentle horizontal drift
            jelly.x += std::cos(m_timeAccumulator * 0.5f + jelly.pulseOffset) * 0.3f * deltaTime;

            // Bounce off top
            if (jelly.y <= 1.0f) {
                jelly.y = 1.0f;
                jelly.verticalDir = VerticalDirection::Down; // Turn downwards
            }
            // Bounce off bottom (keep them floating above the seaweed roots)
            else if (jelly.y >= m_height - 4.0f) {
                jelly.y = m_height - 4.0f;
                jelly.verticalDir = VerticalDirection::Up;
            }
            
            // Keep bounds horizontal
            if (jelly.x <= 1.0f) jelly.x = 1.0f;
            if (jelly.x >= m_width - 3.0f) jelly.x = m_width - 3.0f;
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

        // Draw Seaweed
        for (const auto& weed : m_seaweeds) {
            // Build the seaweed from the bottom up
            for (int i = 0; i < weed.height; ++i) {
                int drawY = (m_height - 2) - i; 
                
                // Calculate a gentle sway using the accumulator and the segment height
                float sway = std::sin(m_timeAccumulator * 2.0f + weed.swayOffset + i * 0.5f);
                
                char c = '|';
                if (sway > 0.4f) c = '/';
                else if (sway < -0.4f) c = '\\';
                
                grid.SetCell(weed.x, drawY, c);
            }
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
            
            // Add a tiny sine wobble to the actual Y coordinate just to make their tails look alive
            int iy = static_cast<int>(fish.y + std::sin(m_timeAccumulator * 4.0f + fish.wobbleOffset) * 0.3f); 
            
            if (ix > 0 && ix < m_width - 3 && iy > 0 && iy < m_height - 1) {
                if (fish.direction == Direction::Right) { 
                    grid.SetCell(ix, iy, '>');
                    grid.SetCell(ix + 1, iy, '<');
                    grid.SetCell(ix + 2, iy, '>');
                } else { 
                    grid.SetCell(ix, iy, '<');
                    grid.SetCell(ix + 1, iy, '>');
                    grid.SetCell(ix + 2, iy, '<');
                }
            }
        }

        // Jellyfish - drawn as a 3x2 sprite
        // (o)
        // / \ .
        for (const auto& jelly : m_jellyfishes) {
            int ix = static_cast<int>(jelly.x);
            int iy = static_cast<int>(jelly.y);
            
            if (ix > 0 && ix < m_width - 3 && iy > 0 && iy < m_height - 2) {
                
                // Determine animation frame based on the pulse sine wave!
                float pulse = std::sin(m_timeAccumulator * 3.0f + jelly.pulseOffset);
                char tentacleLeft = (pulse > 0.0f) ? '/' : '|';
                char tentacleRight = (pulse > 0.0f) ? '\\' : '|';

                // Top bell
                grid.SetCell(ix, iy, '(');
                grid.SetCell(ix + 1, iy, 'o');
                grid.SetCell(ix + 2, iy, ')');
                
                // Animated tentacles
                grid.SetCell(ix, iy + 1, tentacleLeft);
                grid.SetCell(ix + 2, iy + 1, tentacleRight);
            }
        }
    }

} // namespace Aquarium::Worlds