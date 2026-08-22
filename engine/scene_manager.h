#pragma once

#include "engine/grid.h"
#include "engine/scene.h"
#include <memory>

namespace ASCIIpaper::Engine {

class SceneManager {
  public:
    SceneManager() = default;

    // Returns a raw pointer so outside classes can
    // look at the scene without stealing ownership of it
    Scene* GetCurrentScene() const {
        return m_currentScene.get();
    }

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