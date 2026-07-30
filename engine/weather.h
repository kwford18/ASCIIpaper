#pragma once
#include <vector>
#include <cstdint>
#include "engine/grid.h"

namespace ASCIIpaper::Engine {

    enum class WeatherType {
        None,
        Rain,
        Snow
    };

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
        void Draw(CharacterGrid& grid);

    private:
        WeatherType m_type = WeatherType::None;
        int m_width = 0;
        int m_height = 0;
        std::vector<Particle> m_particles;
    };

} // namespace ASCIIpaper::Engine