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

        int particleCount = (type == WeatherType::Rain) ? 150 : 100;
        
        // Rain is fast and falls slightly right; Snow is slow and drifts
        std::uniform_real_distribution<float> speedDist = 
            (type == WeatherType::Rain) ? std::uniform_real_distribution<float>(30.0f, 50.0f) 
                                        : std::uniform_real_distribution<float>(5.0f, 15.0f);
                                        
        std::uniform_real_distribution<float> driftDist = 
            (type == WeatherType::Rain) ? std::uniform_real_distribution<float>(2.0f, 8.0f) 
                                        : std::uniform_real_distribution<float>(-5.0f, 5.0f);

        for (int i = 0; i < particleCount; ++i) {
            char sym = (type == WeatherType::Rain) ? '|' : '*';
            if (type == WeatherType::Snow && i % 2 == 0) sym = '.'; // Mix small and large snowflakes
            
            uint8_t c = (type == WeatherType::Rain) ? 120 : 255; 
            
            m_particles.push_back({xDist(rng), yDist(rng), speedDist(rng), driftDist(rng), sym, c, c, c});
        }
    }

    void WeatherSystem::Update(float deltaTime) {
        if (m_type == WeatherType::None) return;

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
    }

    void WeatherSystem::Draw(CharacterGrid& grid) {
        if (m_type == WeatherType::None) return;

        for (const auto& p : m_particles) {
            int ix = static_cast<int>(p.x);
            int iy = static_cast<int>(p.y);
            if (ix >= 0 && ix < m_width && iy >= 0 && iy < m_height) {
                // Weather falls behind the foreground elements
                if (grid.GetCell(ix, iy).character == ' ') {
                    grid.SetCell(ix, iy, p.symbol, p.r, p.g, p.b);
                }
            }
        }
    }

} // namespace ASCIIpaper::Engine