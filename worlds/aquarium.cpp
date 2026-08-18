#include <cmath>
#include <string>
#include <algorithm>

#include "engine/types.h"
#include "worlds/aquarium.h"

namespace ASCIIpaper::Worlds {

    AquariumScene::AquariumScene(int width, int height, int fishCount, int bubbleCount, int jellyCount, bool systemSync) 
        : m_width(width), m_height(height), m_timeAccumulator(0.0f), m_baseFishCount(fishCount), m_systemSync(systemSync), m_rng(std::random_device{}()) {
        InitializeWorld(fishCount, bubbleCount, jellyCount);
    }

    void AquariumScene::ClearEntities() {
        m_fishes.clear();
        m_bubbles.clear();
        m_seaweeds.clear();
        m_jellyfishes.clear();
        m_corals.clear();
        m_shrimps.clear();
    }

    void AquariumScene::InitializeWorld(int fishCount, int bubbleCount, int jellyCount) {
        // Setup random distributions to keep entities inside the boundaries
        // Setup random distributions to keep entities inside the boundaries
        float maxX = std::max(2.1f, static_cast<float>(m_width - 5.0f));
        std::uniform_real_distribution<float> xDist(2.0f, maxX);

        float maxY = std::max(4.1f, static_cast<float>(m_height - 10.0f));
        std::uniform_real_distribution<float> yDist(4.0f, maxY); // Keep them away from extreme edges

        std::uniform_real_distribution<float> fSpeedDist(2.0f, 5.0f);
        std::uniform_real_distribution<float> phaseDist(0.0f, 6.28f); // 0 to 2*PI for offsets
        std::uniform_int_distribution<int> dirDist(0, 1);

        // For random wandering
        std::uniform_real_distribution<float> vyDist(-1.5f, 1.5f);
        std::uniform_real_distribution<float> timerDist(1.0f, 5.0f);

        // For color
        std::uniform_int_distribution<int> colorDist(100, 255);

        // Spawn Fish with wobble offsets
        for (int i = 0; i < fishCount; ++i) {
            float vx = fSpeedDist(m_rng);
            Engine::Direction dir = dirDist(m_rng) == 0 ? Engine::Direction::Left : Engine::Direction::Right;
            if (dir == Engine::Direction::Left) vx = -vx;

            m_fishes.push_back({
                xDist(m_rng), yDist(m_rng), vx, vyDist(m_rng), dir, phaseDist(m_rng), timerDist(m_rng),
                // Assign random colors!
                static_cast<uint8_t>(colorDist(m_rng)), 
                static_cast<uint8_t>(colorDist(m_rng)), 
                static_cast<uint8_t>(colorDist(m_rng)),
                false // isLeaving
            });
        }

        std::uniform_real_distribution<float> bSpeedDist(1.0f, 3.0f);
        std::uniform_int_distribution<int> sizeDist(0, 1);

        // Spawn bubbles
        for (int i = 0; i < bubbleCount; ++i) {
            m_bubbles.push_back({
                xDist(m_rng), yDist(m_rng), bSpeedDist(m_rng), sizeDist(m_rng) == 0 ? 'o' : 'O'
            });
        }

        // Floor Occupation Tracker to prevent overlapping
        std::vector<bool> xOccupied(m_width, false);

        // Distribtions for seaweed placement and height
        std::uniform_int_distribution<int> swXDist(2, m_width - 3);
        std::uniform_int_distribution<int> swHeightDist(3, 8); 
        std::uniform_real_distribution<float> swOffsetDist(0.0f, 6.28f);

        // Plant 6 stalks of seaweed at the bottom of the tank
        for (int i = 0; i < 6; ++i) {
            int x = swXDist(m_rng);
            while (xOccupied[x]) x = swXDist(m_rng); // Find an empty spot
            xOccupied[x] = true;

            m_seaweeds.push_back({ x, swHeightDist(m_rng), swOffsetDist(m_rng) });
        }

        std::uniform_int_distribution<int> coralColorDist(150, 255);
        std::uniform_int_distribution<int> coralTypeDist(0, 2);
        for(int i = 0; i < 12; ++i) {
            int x = swXDist(m_rng);
            int attempts = 0;
            while (xOccupied[x] && attempts < 10) { // Don't infinite loop if crowded
                x = swXDist(m_rng);
                attempts++;
            }
            if (xOccupied[x]) continue; // Skip if we couldn't find a spot
            xOccupied[x] = true;

            char sym = '&';
            if (coralTypeDist(m_rng) == 1) sym = '%';
            else if (coralTypeDist(m_rng) == 2) sym = 'Y';
            
            m_corals.push_back({
                x, m_height - 5, sym,
                static_cast<uint8_t>(coralColorDist(m_rng)), // Bright Reds/Pinks
                static_cast<uint8_t>(coralColorDist(m_rng) / 2),
                static_cast<uint8_t>(coralColorDist(m_rng) / 2) 
            });
        }

        // Spawn jellyfish with random positions, speeds, and pulse offsets
        std::uniform_real_distribution<float> jSpeedDist(0.5f, 1.5f);
        for (int i = 0; i < jellyCount; ++i) {
            m_jellyfishes.push_back({
                xDist(m_rng), yDist(m_rng), jSpeedDist(m_rng), phaseDist(m_rng),
                Engine::VerticalDirection::Up // Start by floating upwards
            });
        }

        // Spawn shrimp
        std::uniform_real_distribution<float> sSpeedDist(0.5f, 2.0f);
        for (int i = 0; i < 12; ++i) {
            char sym = (dirDist(m_rng) == 0) ? 'j' : ',';
            m_shrimps.push_back({
                xDist(m_rng), yDist(m_rng), sSpeedDist(m_rng), sSpeedDist(m_rng), phaseDist(m_rng), sym
            });
        }

        // Spawn crab
        m_crab = {
            static_cast<float>(m_width / 2),    // Start in the middle
            static_cast<float>(m_height - 4),   // Rest on the very bottom row
            8.0f,                               // Scurry speed
            Engine::Direction::Right,
            0.0f,                               // Timer
            true,                               // isMoving
            220, 200, 180                       // Tan shell color
        };

        // Spawn whale (starts inactive; real spawn position set when it activates in Update)
        m_whale = {
            static_cast<float>(m_width) + 80.0f,
            static_cast<float>(m_height / 4),
            -12.0f, false, 0.0f
        };
    }

    // ========== UPDATE LOGIC ==========
    void AquariumScene::Update(float deltaTime) {
        float cpuMultiplier = 1.0f;
        float ramMultiplier = 1.0f;

        UpdateSystemMonitor(deltaTime, cpuMultiplier, ramMultiplier);

        // Apply RAM Multiplier to our global time to speed up wave physics
        m_timeAccumulator += deltaTime * ramMultiplier;

        UpdateFishes(deltaTime, cpuMultiplier);
        UpdateShrimps(deltaTime, cpuMultiplier);
        UpdateBubbles(deltaTime);
        UpdateJellyfishes(deltaTime, cpuMultiplier);
        UpdateCrab(deltaTime, cpuMultiplier);
        UpdateWhale(deltaTime, cpuMultiplier);
    }

    void AquariumScene::UpdateSystemMonitor(float deltaTime, float& cpuMultiplier, float& ramMultiplier) {
        /*
         * Poll the system monitor if sync is enabled
         * CPU usage scales the movement speed of all living creatures.
         * RAM usage scales the water current (bubbles and seaweed sway),
         * and dynamically injects or flags fish for despawning!
        */
        if (m_systemSync) {
            m_sysMonitor.Update(deltaTime);
            float currentCpu = m_sysMonitor.GetCpuUsage();
            float currentRam = m_sysMonitor.GetRamUsage();

            cpuMultiplier = 1.0f + (currentCpu / 33.3f); 
            ramMultiplier = 1.0f + (currentRam / 50.0f); 

            // Calculate target fish: Base count + up to 20 extra fish at 100% RAM
            int targetFishCount = m_baseFishCount + static_cast<int>(currentRam / 5.0f);
            
            int activeFish = 0;
            for (const auto& fish : m_fishes) {
                if (!fish.isLeaving) activeFish++;
            }

            if (activeFish < targetFishCount) {
                // Spawn a new fish just off screen
                float maxY = std::max(4.1f, static_cast<float>(m_height - 10.0f));
                std::uniform_real_distribution<float> yDist(4.0f, maxY);

                std::uniform_real_distribution<float> fSpeedDist(2.0f, 5.0f);
                std::uniform_real_distribution<float> phaseDist(0.0f, 6.28f);
                std::uniform_real_distribution<float> timerDist(1.0f, 5.0f);
                std::uniform_int_distribution<int> colorDist(100, 255);
                std::uniform_int_distribution<int> dirDist(0, 1);

                Engine::Direction dir = dirDist(m_rng) == 0 ? Engine::Direction::Left : Engine::Direction::Right;
                float vx = fSpeedDist(m_rng);
                if (dir == Engine::Direction::Left) vx = -vx;
                
                float startX = (dir == Engine::Direction::Right) ? -5.0f : static_cast<float>(m_width + 5);

                m_fishes.push_back({
                    startX, yDist(m_rng), vx, 0.0f, dir, phaseDist(m_rng), timerDist(m_rng),
                    static_cast<uint8_t>(colorDist(m_rng)), 
                    static_cast<uint8_t>(colorDist(m_rng)), 
                    static_cast<uint8_t>(colorDist(m_rng)),
                    false
                });
            } else if (activeFish > targetFishCount) {
                // Flag one fish to gracefully leave the tank
                for (auto& fish : m_fishes) {
                    if (!fish.isLeaving) {
                        fish.isLeaving = true;
                        break;
                    }
                }
            }
        }
    }

    void AquariumScene::UpdateFishes(float deltaTime, float cpuMultiplier) {
        // Move Fish (iterator)
        for (auto it = m_fishes.begin(); it != m_fishes.end(); ) {
            it->changeTimer -= deltaTime;
            if (it->changeTimer <= 0.0f) {
                std::uniform_real_distribution<float> vyDist(-1.5f, 1.5f);
                std::uniform_real_distribution<float> timerDist(2.0f, 6.0f);
                it->vy = vyDist(m_rng);
                it->changeTimer = timerDist(m_rng);
            }

            // Apply CPU Multiplier to swim speed
            it->x += it->vx * cpuMultiplier * deltaTime;
            it->y += it->vy * cpuMultiplier * deltaTime;
            
            if (it->isLeaving) {
                if (it->x < -10.0f || it->x > m_width + 10.0f) {
                    it = m_fishes.erase(it);
                    continue;
                }
            } else {
                // Checking vx ensures dynamically spawned fish can smoothly swim in from off-screen
                if (it->x <= 1.0f && it->vx < 0.0f) {
                    it->x = 1.0f;
                    it->vx = std::abs(it->vx); 
                    it->direction = Engine::Direction::Right; 
                } else if (it->x >= m_width - 4.0f && it->vx > 0.0f) {
                    it->x = m_width - 4.0f;
                    it->vx = -std::abs(it->vx); 
                    it->direction = Engine::Direction::Left;  
                }

                if (it->y <= 1.0f && it->vy < 0.0f) {
                    it->y = 1.0f;
                    it->vy = std::abs(it->vy); 
                } else if (it->y >= m_height - 6.0f && it->vy > 0.0f) { 
                    it->y = m_height - 6.0f;
                    it->vy = -std::abs(it->vy); 
                }
            }
            ++it;
        }
    }

    void AquariumScene::UpdateShrimps(float deltaTime, float cpuMultiplier) {
        // Move Shrimp
        for (auto& shrimp : m_shrimps) {
            shrimp.x += shrimp.vx * cpuMultiplier * deltaTime;
            shrimp.y += shrimp.vy * cpuMultiplier * deltaTime;

            if (shrimp.x <= 1.0f || shrimp.x >= m_width - 2.0f) shrimp.vx = -shrimp.vx;
            if (shrimp.y <= 1.0f || shrimp.y >= m_height - 4.0f) shrimp.vy = -shrimp.vy;
        }
    }

    void AquariumScene::UpdateBubbles(float deltaTime) {
        // Move Bubbles
        for (auto& bubble : m_bubbles) {
            bubble.y -= bubble.speed * deltaTime;

            // If a bubble hits the top, respawn it at the bottom with a new random x position
            if (bubble.y <= 1.0f) {
                bubble.y = static_cast<float>(m_height - 2);
                float maxX = std::max(1.1f, static_cast<float>(m_width - 2.0f));
                std::uniform_real_distribution<float> xDist(1.0f, maxX);
                bubble.x = xDist(m_rng);
            }
        }
    }

    void AquariumScene::UpdateJellyfishes(float deltaTime, float cpuMultiplier) {
        // Move jellyfish
        for (auto& jelly : m_jellyfishes) {
            // Pulse logic
            float pulse = std::sin(m_timeAccumulator * 3.0f + jelly.pulseOffset);
            float pulseMultiplier = 1.0f + 0.8f * pulse;
            
            // Apply speed combined with their current vertical direction
            jelly.y += jelly.speed * pulseMultiplier * cpuMultiplier * static_cast<float>(jelly.verticalDir) * deltaTime;
            
            // Gentle horizontal drift
            jelly.x += std::cos(m_timeAccumulator * 0.5f + jelly.pulseOffset) * 0.3f * deltaTime;

            // Bounce off top
            if (jelly.y <= 1.0f) {
                jelly.y = 1.0f;
                jelly.verticalDir = Engine::VerticalDirection::Down; // Turn downwards
            }
            // Bounce off bottom (keep them floating above the seaweed roots)
            else if (jelly.y >= m_height - 4.0f) {
                jelly.y = m_height - 4.0f;
                jelly.verticalDir = Engine::VerticalDirection::Up;
            }
            
            // Keep bounds horizontal
            if (jelly.x <= 1.0f) jelly.x = 1.0f;
            if (jelly.x >= m_width - 3.0f) jelly.x = m_width - 3.0f;
        }
    }

    void AquariumScene::UpdateCrab(float deltaTime, float cpuMultiplier) {
        /*
         * Update the Hermit Crab's state machine
         * The crab will scurry across the floor for a few seconds
         * then pause to rest. When it wakes up, it may flip directions
         */
        m_crab.timer += deltaTime;
        if (m_crab.isMoving) {
            m_crab.x += m_crab.speed * static_cast<float>(m_crab.direction) * cpuMultiplier * deltaTime;
            if (m_crab.timer > 4.0f) {
                m_crab.isMoving = false;
                m_crab.timer = 0.0f;
            }
            
            if (m_crab.x > m_width - 4) m_crab.direction = Engine::Direction::Left;
            else if (m_crab.x < 2) m_crab.direction = Engine::Direction::Right;
            
        } else {
            if (m_crab.timer > 2.0f) {
                m_crab.isMoving = true;
                m_crab.timer = 0.0f;
                
                std::uniform_int_distribution<int> dirDist(0, 1);
                if (dirDist(m_rng) == 0) {
                    m_crab.direction = (m_crab.direction == Engine::Direction::Right) ? 
                        Engine::Direction::Left : Engine::Direction::Right;
                }
            }
        }
    }

    void AquariumScene::UpdateWhale(float deltaTime, float cpuMultiplier) {
        // Background whale
        if (!m_whale.active) {
            m_whale.timer += deltaTime;
            if (m_whale.timer > 45.0f) { 
                m_whale.active = true;
                m_whale.x = static_cast<float>(m_width) + 60.0f; // Start safely offscreen to the right
                
                // Randomize its vertical swimming lane
                float maxWhaleY = std::max(5.1f, static_cast<float>(m_height) / 2.0f);
                std::uniform_real_distribution<float> whaleYDist(5.0f, maxWhaleY);
                m_whale.y = whaleYDist(m_rng);
                
                m_whale.timer = 0.0f;
            }
        } else {
            // Whale speed is also dictated by your CPU usage! (speed is negative, so it drifts left)
            m_whale.x += m_whale.speed * cpuMultiplier * deltaTime;
            if (m_whale.x < -80.0f) {
                m_whale.active = false; 
            }
        }
    }

    // ========== DRAW LOGIC ==========
    void AquariumScene::Draw(Engine::CharacterGrid& grid) {
        grid.Clear();
        
        DrawBackground(grid);
        DrawEntities(grid);
        DrawForeground(grid);
        DrawHUD(grid);
    }

    void AquariumScene::DrawBackground(Engine::CharacterGrid& grid) {
        /*
         * Draw the background Whale.
         * Draw this BEFORE the seaweed, fish, and bubbles so that
         * it appears to be swimming in the deep background of the tank.
         */
        if (m_whale.active) {
            std::vector<std::string> whaleArt = {
                "       .",
                "      \":\" ",
                "    ___:____     |\"\\/\"|",
                "  ,'        `.    \\  /",
                "  |  O        \\___/  |",
                "   \\________________/"
            };
            
            int wx = static_cast<int>(m_whale.x);
            int wy = static_cast<int>(m_whale.y);
            
            for (size_t r = 0; r < whaleArt.size(); ++r) {
                for (size_t c = 0; c < whaleArt[r].length(); ++c) {
                    if (whaleArt[r][c] != ' ' && (wx + static_cast<int>(c)) >= 0 && (wx + static_cast<int>(c)) < m_width) {
                        // Dark blue/grey to look distant and massive
                        grid.SetCell(wx + static_cast<int>(c), wy + static_cast<int>(r), whaleArt[r][c], 40, 60, 100); 
                    }
                }
            }
        }

        // Draw Seaweed
        for (const auto& weed : m_seaweeds) {
            // Build the seaweed from the bottom up
            for (int i = 0; i < weed.height; ++i) {
                // Seaweed now safely roots at m_height - 5
                int drawY = (m_height - 5) - i; 
                
                // Calculate a gentle sway using the accumulator and the segment height
                float sway = std::sin(m_timeAccumulator * 2.0f + weed.swayOffset + i * 0.5f);
                
                char c = '|';
                if (sway > 0.4f) c = '/';
                else if (sway < -0.4f) c = '\\';
                
                grid.SetCell(weed.x, drawY, c, 50, 205, 50);
            }
        }

        // Draw coral
        for (const auto& coral : m_corals) {
            grid.SetCell(coral.x, coral.y, coral.symbol, coral.r, coral.g, coral.b);
        }

        // Draw sandy floor
        for (int y = m_height - 4; y < m_height; ++y) {
            for (int x = 0; x < m_width; ++x) {
                // Darken the sand slightly as it gets deeper
                uint8_t colorOffset = static_cast<uint8_t>((y - (m_height - 4)) * 10);
                grid.SetCell(x, y, '.', 200 - colorOffset, 180 - colorOffset, 100 - colorOffset);
            }
        }
    }

    void AquariumScene::DrawEntities(Engine::CharacterGrid& grid) {
        // Draw Bubbles
        for (const auto& bubble : m_bubbles) {
            int ix = static_cast<int>(bubble.x);
            int iy = static_cast<int>(bubble.y);
            grid.SetCell(ix, iy, bubble.symbol, 135, 206, 235);
        }

        // Draw Fish
        for (const auto& fish : m_fishes) {
            int ix = static_cast<int>(fish.x);
            
            // Add a tiny sine wobble to the actual Y coordinate just to make their tails look alive
            int iy = static_cast<int>(fish.y + std::sin(m_timeAccumulator * 4.0f + fish.wobbleOffset) * 0.3f); 
            
            if (ix > 0 && ix < m_width - 3 && iy > 0 && iy < m_height - 1) {
                if (fish.direction == Engine::Direction::Right) { 
                    grid.SetCell(ix, iy, '>', fish.r, fish.g, fish.b);
                    grid.SetCell(ix + 1, iy, '<', fish.r, fish.g, fish.b);
                    grid.SetCell(ix + 2, iy, '>', fish.r, fish.g, fish.b);
                } else { 
                    grid.SetCell(ix, iy, '<', fish.r, fish.g, fish.b);
                    grid.SetCell(ix + 1, iy, '>', fish.r, fish.g, fish.b);
                    grid.SetCell(ix + 2, iy, '<', fish.r, fish.g, fish.b);
                }
            }
        }

        // Draw Shrimp
        for (const auto& shrimp : m_shrimps) {
            int ix = static_cast<int>(shrimp.x);
            int iy = static_cast<int>(shrimp.y + std::sin(m_timeAccumulator * 6.0f + shrimp.wobbleOffset) * 0.5f);
            if (ix >= 0 && ix < m_width && iy >= 0 && iy < m_height - 2) {
                grid.SetCell(ix, iy, shrimp.symbol, 255, 120, 100); // Shrimp pink/orange
            }
        }

        /*
         * Jellyfish - drawn as a 3x2 sprite
         * (o)
         * / \ .
         */
        for (const auto& jelly : m_jellyfishes) {
            int ix = static_cast<int>(jelly.x);
            int iy = static_cast<int>(jelly.y);
            
            if (ix > 0 && ix < m_width - 3 && iy > 0 && iy < m_height - 2) {
                
                // Determine animation frame based on the pulse sine wave!
                float pulse = std::sin(m_timeAccumulator * 3.0f + jelly.pulseOffset);
                char tentacle = (pulse > 0.0f) ? '|' : '~';

                // Top bell
                grid.SetCell(ix, iy, '(', 218, 112, 214);
                grid.SetCell(ix + 1, iy, '_', 218, 112, 214);
                grid.SetCell(ix + 2, iy, ')', 218, 112, 214);
                
                // Animated tentacles
                grid.SetCell(ix, iy + 1, tentacle, 218, 112, 214);
                grid.SetCell(ix + 2, iy + 1, tentacle, 218, 112, 214);
            }
        }
    }

    void AquariumScene::DrawForeground(Engine::CharacterGrid& grid) {
        // Hermit crab  
        int cx = static_cast<int>(m_crab.x);
        int cy = static_cast<int>(m_crab.y);
        if (cx >= 0 && cx < m_width - 2) {
            if (m_crab.direction == Engine::Direction::Right) {
                // Shell on the left, pincers on the right: @<
                grid.SetCell(cx, cy, '@', m_crab.r, m_crab.g, m_crab.b);
                grid.SetCell(cx + 1, cy, '<', 255, 100, 100); // Red claws
            } else {
                // Pincers on the left, shell on the right: >@
                grid.SetCell(cx, cy, '>', 255, 100, 100); // Red claws
                grid.SetCell(cx + 1, cy, '@', m_crab.r, m_crab.g, m_crab.b);
            }
        }
    }

    void AquariumScene::DrawHUD(Engine::CharacterGrid& grid) {
        // Draw Debug HUD
        if (m_systemSync) {
            std::string hudStr = "SYSTEM SYNC | CPU: " + std::to_string(static_cast<int>(m_sysMonitor.GetCpuUsage())) + 
                                 "% | RAM: " + std::to_string(static_cast<int>(m_sysMonitor.GetRamUsage())) + "%";
            for (size_t i = 0; i < hudStr.length(); ++i) {
                grid.SetCell(2 + static_cast<int>(i), 2, hudStr[i], 0, 255, 255); 
            }
        }
    }

} // namespace ASCIIpaper::Worlds