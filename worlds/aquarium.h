#pragma once
#include "engine/grid.h"

namespace Aquarium::Worlds {

    class AquariumScene {
    public:
        AquariumScene(int width, int height);

        // Processes simulation logic (fish swimming, bubbles rising)
        void Update(float deltaTime);

        // Writes the current state of the simulation into the character grid
        void Draw(Engine::CharacterGrid& grid);

    private:
        int m_width;
        int m_height;
        float m_timeAccumulator;
    };

} // namespace Aquarium::Worlds