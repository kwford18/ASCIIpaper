#pragma once
#include "engine/grid.h"
#include <vector>
#include <random>

namespace Aquarium::Worlds {

    // Strongly-typed enum for direction
    enum class Direction {
        Left = -1,
        Right = 1
    };

    // Data structure for a single fish
    struct Fish {
        float x, y;
        float speed;
        Direction direction;
    };

    // Data structure for a single bubble
    struct Bubble {
        float x, y;
        float speed;
        char symbol; // 'o' or 'O'
    };

    class AquariumScene {
    public:
        AquariumScene(int width, int height);

        // Processes simulation logic (fish swimming, bubbles rising)
        void Update(float deltaTime);

        // Draw the current state of the simulation into the character grid
        void Draw(Engine::CharacterGrid& grid);

        // Clears all entities from the scene
        void ClearEntities();

        // Exposed strictly so our unit tests can verify spawning
        size_t GetFishCount() const { return m_fishes.size(); }
        size_t GetBubbleCount() const { return m_bubbles.size(); }

    private:
        int m_width;
        int m_height;
        float m_timeAccumulator;

        // Entity lists
        std::vector<Fish> m_fishes;
        std::vector<Bubble> m_bubbles;
        
        // Standard C++ random number generator
        std::mt19937 m_rng;
        
        // Helper to spawn initial entities
        void InitializeWorld();
    };

} // namespace Aquarium::Worlds