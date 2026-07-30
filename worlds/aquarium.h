#pragma once
#include <vector>
#include <random>
#include "engine/grid.h"
#include "engine/scene.h"
#include "engine/types.h"
#include "engine/system_monitor.h"

namespace ASCIIpaper::Worlds {

    // Data structure for a single fish
    struct Fish {
        float x, y;
        float vx, vy;         
        Engine::Direction direction;
        float wobbleOffset;
        float changeTimer;    
        uint8_t r, g, b;
    };

    // Data structure for a single jellyfish
    struct Jellyfish {
        float x, y;
        float speed;
        float pulseOffset;
        Engine::VerticalDirection verticalDir;
    };

    // Data structure for a single bubble
    struct Bubble {
        float x, y;
        float speed;
        char symbol; // 'o' or 'O'
    };

    // Data structure for a single seaweed
    struct Seaweed {
        int x;
        int height;
        float swayOffset; // Random offset so they don't sway in unison
    };

    // Data structure for coral
    struct Coral {
        int x, y;
        char symbol;
        uint8_t r, g, b;
    };

    // Data structure for hermit crab
    struct HermitCrab {
        float x, y;
        float speed;
        Engine::Direction direction;
        float timer;
        bool isMoving;
        uint8_t r, g, b;
    };

    // Data structure for whale
    struct Whale {
        float x, y;
        float speed;
        bool active;
        float timer;
    };
    

    class AquariumScene : public Engine::Scene {
    public:
        AquariumScene(int width, int height, int fishCount, int bubbleCount, int jellyCount, bool systemSync);

        // Processes simulation logic (fish swimming, bubbles rising)
        void Update(float deltaTime) override;

        // Draw the current state of the simulation into the character grid
        void Draw(Engine::CharacterGrid& grid) override;

        // Clears all entities from the scene
        void ClearEntities();

        // Exposed strictly so our unit tests can verify spawning
        size_t GetFishCount() const { return m_fishes.size(); }
        size_t GetBubbleCount() const { return m_bubbles.size(); }

    private: 
        int m_width;
        int m_height;
        float m_timeAccumulator;

        bool m_systemSync;
        Engine::SystemMonitor m_sysMonitor;

        // Entity lists
        std::vector<Fish> m_fishes;
        std::vector<Bubble> m_bubbles;
        std::vector<Seaweed> m_seaweeds;
        std::vector<Jellyfish> m_jellyfishes;
        std::vector<Coral> m_corals;

        HermitCrab m_crab;
        Whale m_whale;
        
        // RNG
        std::mt19937 m_rng;
        
        // Helper to spawn initial entities
        void InitializeWorld(int fishCount, int bubbleCount, int jellyCount);
    };

} // namespace ASCIIpaper::Worlds