#include "engine/timer.h"

namespace Aquarium::Engine {

    Timer::Timer(int targetFps) : m_targetFps(targetFps), m_deltaTime(0.0f) {
        // Calculate how many CPU ticks a single frame should take
        m_targetFrameTicks = SDL_GetPerformanceFrequency() / m_targetFps;
        m_lastTime = SDL_GetPerformanceCounter();
    }

    void Timer::Start() {
        m_lastTime = SDL_GetPerformanceCounter();
    }

    void Timer::Tick() {
        Uint64 currentTime = SDL_GetPerformanceCounter();
        Uint64 elapsedTicks = currentTime - m_lastTime;

        // If the frame finished too fast, delay the thread to hit our target FPS
        if (elapsedTicks < m_targetFrameTicks) {
            Uint64 delayTicks = m_targetFrameTicks - elapsedTicks;
            float delayMs = (float)(delayTicks * 1000) / (float)SDL_GetPerformanceFrequency();
            SDL_Delay((Uint32)delayMs);
            
            // Recalculate after the delay
            currentTime = SDL_GetPerformanceCounter();
            elapsedTicks = currentTime - m_lastTime;
        }

        m_deltaTime = (float)elapsedTicks / (float)SDL_GetPerformanceFrequency();
        m_lastTime = currentTime;
    }

    float Timer::GetDeltaTime() const {
        return m_deltaTime;
    }

} // namespace Aquarium::Engine