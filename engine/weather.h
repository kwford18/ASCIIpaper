#pragma once
#include "engine/grid.h"
#include <cstdint>
#include <vector>

namespace ASCIIpaper::Engine {

enum class WeatherType { None, Rain, Snow, Storm };

struct Particle {
    float x, y;
    float speedY, speedX;
    char symbol;
    uint8_t r, g, b;
};

class WeatherSystem {
  public:
    WeatherSystem() = default;

    void Initialize(WeatherType type, int width, int height);
    void Update(float deltaTime);
    void Draw(CharacterGrid& grid, int maxDrawY = -1);

    void SetLightningFrequency(float minSeconds, float maxSeconds) {
        m_lightningMin = minSeconds;
        m_lightningMax = maxSeconds;
    }

  private:
    WeatherType m_type = WeatherType::None;
    int m_width = 0;
    int m_height = 0;
    std::vector<Particle> m_particles;

    // Lightning
    float m_lightningTimer = 0.0f;
    float m_lightningThreshold = 5.0f;
    bool m_isLightning = false;
    int m_lightningX = 0;
    float m_lightningMin = 5.0f;
    float m_lightningMax = 15.0f;
    std::vector<std::pair<int, int>> m_currentBolt;
};

} // namespace ASCIIpaper::Engine