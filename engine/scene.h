#pragma once

#include "engine/grid.h"

namespace ASCIIpaper::Engine {

class Scene {
  public:
    // A virtual destructor ensures derived scenes clean up their memory properly
    virtual ~Scene() = default;

    // Called every frame to process simulation logic
    virtual void Update(float deltaTime) = 0;

    // Called every frame to draw the current state into the character grid
    virtual void Draw(CharacterGrid& grid) = 0;
};

} // namespace ASCIIpaper::Engine