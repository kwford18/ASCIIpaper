#pragma once

#include "engine/scene.h"

namespace Aquarium::Worlds {

    class CityScene : public Engine::Scene {
    public:
        CityScene(int width, int height);

        void Update(float deltaTime) override;
        void Draw(Engine::CharacterGrid& grid) override;

    private:
        int m_width;
        int m_height;
        float m_timeAccumulator;
    };

} // namespace Aquarium::Worlds