#include "engine/scene_manager.h"

namespace Aquarium::Engine {

    void SceneManager::ChangeScene(std::unique_ptr<Scene> newScene) {
        // std::move transfers ownership from the caller to the manager
        m_currentScene = std::move(newScene);
    }

    void SceneManager::Update(float deltaTime) {
        if (m_currentScene) {
            m_currentScene->Update(deltaTime);
        }
    }

    void SceneManager::Draw(CharacterGrid& grid) {
        if (m_currentScene) {
            m_currentScene->Draw(grid);
        }
    }

} // namespace Aquarium::Engine