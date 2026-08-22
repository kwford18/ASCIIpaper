#pragma once
#include <SDL3/SDL.h>

namespace ASCIIpaper::Engine {

class Timer {
  public:
    Timer(int targetFps);

    void Start();

    // Pauses the thread to enforce the target framerate and calculates delta time
    void Tick();

    // Returns the time passed since the last frame in seconds
    float GetDeltaTime() const;

  private:
    int m_targetFps;
    Uint64 m_targetFrameTicks;
    Uint64 m_lastTime;
    float m_deltaTime;
};

} // namespace ASCIIpaper::Engine