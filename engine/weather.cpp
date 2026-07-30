#include "engine/weather.h"
#include <random>

namespace ASCIIpaper::Engine {

    void WeatherSystem::Initialize(WeatherType type, int width, int height) {
        m_type = type;
        m_width = width;
        m_height = height;
        m_particles.clear();

        if (type == WeatherType::None) return;

        std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> xDist(0.0f, static_cast<float>(width));
        std::uniform_real_distribution<float> yDist(0.0f, static_cast<float>(height));

        // Heavy rain for storms
        int particleCount = (type == WeatherType::Storm) ? 200 : ((type == WeatherType::Rain) ? 150 : 100); 
        
        // Rain is fast and falls slightly right; Snow is slow and drifts
        std::uniform_real_distribution<float> speedDist = 
            (type == WeatherType::Rain || type == WeatherType::Storm) ? std::uniform_real_distribution<float>(30.0f, 50.0f) 
                                                                      : std::uniform_real_distribution<float>(5.0f, 15.0f);
                                        
        std::uniform_real_distribution<float> driftDist = 
            (type == WeatherType::Rain || type == WeatherType::Storm) ? std::uniform_real_distribution<float>(2.0f, 8.0f) 
                                                                      : std::uniform_real_distribution<float>(-5.0f, 5.0f);

        for (int i = 0; i < particleCount; ++i) {
            char sym = (type == WeatherType::Rain || type == WeatherType::Storm) ? '|' : '*';
            if (type == WeatherType::Snow && i % 2 == 0) sym = '.'; // Mix small and large snowflakes
            
            uint8_t c = (type == WeatherType::Rain || type == WeatherType::Storm) ? 120 : 255; 
            
            m_particles.push_back({xDist(rng), yDist(rng), speedDist(rng), driftDist(rng), sym, c, c, c});
        }
    }

    void WeatherSystem::Update(float deltaTime) {
        if (m_type == WeatherType::None) return;

        // Update Particles
        for (auto& p : m_particles) {
            p.y += p.speedY * deltaTime;
            p.x += p.speedX * deltaTime;
            
            // Screen wrapping
            if (p.y > static_cast<float>(m_height)) {
                p.y = 0.0f;
                std::mt19937 rng(std::random_device{}());
                std::uniform_real_distribution<float> xDist(0.0f, static_cast<float>(m_width));
                p.x = xDist(rng);
            }
            if (p.x < 0.0f) p.x += m_width;
            if (p.x >= static_cast<float>(m_width)) p.x -= m_width;
        }

        // Lightning for storms
        if (m_type == WeatherType::Storm) {
            m_lightningTimer += deltaTime;

            if (m_isLightning) {
                if (m_lightningTimer > 0.15f) { // Flash lasts 0.15 seconds
                    m_isLightning = false;
                    m_lightningTimer = 0.0f;
                }
            } else {
                if (m_lightningTimer > m_lightningThreshold) {
                    m_isLightning = true;
                    m_lightningTimer = 0.0f;
                    
                    // Use the configurable limits for lightning duration
                    m_lightningThreshold = m_lightningMin + static_cast<float>(rand()) / 
                        (static_cast<float>(RAND_MAX / (m_lightningMax - m_lightningMin)));
                    
                    // Generate a jagged bolt
                    m_lightningX = rand() % (m_width - 10) + 5;
                    m_currentBolt.clear();
                    int currentX = m_lightningX;
                    
                    for (int y = 0; y < m_height; ++y) {
                        m_currentBolt.push_back({currentX, y});
                        if (y % 2 == 0) currentX += (rand() % 3) - 1; // zig-zag
                    }
                }
            }
        }
    }

    void WeatherSystem::Draw(CharacterGrid& grid, int maxDrawY) {
        if (m_type == WeatherType::None) return;

        // Determine the cutoff line for the weather
        int bottomLimit = (maxDrawY < 0) ? m_height : maxDrawY;

        // Draw Lightning first so it goes behind buildings/cars but flashes the sky
        if (m_type == WeatherType::Storm && m_isLightning) {

            for (const auto& point : m_currentBolt) {
                if (point.first >= 0 && point.first < m_width && point.second >= 0 && point.second < bottomLimit) {
                    grid.SetCell(point.first, point.second, '|', 255, 255, 200); // Bright white/yellow
                }
            }
        }

        // Draw Particles
        for (const auto& p : m_particles) {
            int ix = static_cast<int>(p.x);
            int iy = static_cast<int>(p.y);
            if (ix >= 0 && ix < m_width && iy >= 0 && iy < bottomLimit) {
                // Weather falls "behind" the foreground elements
                if (grid.GetCell(ix, iy).character == ' ') {
                    grid.SetCell(ix, iy, p.symbol, p.r, p.g, p.b);
                }
            }
        }
    }

} // namespace ASCIIpaper::Engine