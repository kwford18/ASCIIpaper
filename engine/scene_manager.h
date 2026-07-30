#pragma once

#include <memory>
#include "engine/scene.h"
#include "engine/grid.h"

namespace ASCIIpaper::Engine {

    class SceneManager {
    public:
        SceneManager() = default;

        // Takes ownership of a new scene, destroying the old one if it exists
        void ChangeScene(std::unique_ptr<Scene> newScene);

        // Forwards the update and draw calls to the active scene
        void Update(float deltaTime);
        void Draw(CharacterGrid& grid);

    private:
        // unique_ptr guarantees only ONE scene exists at a time and handles deletion
        std::unique_ptr<Scene> m_currentScene;
    };

} // namespace ASCIIpaper::Engine