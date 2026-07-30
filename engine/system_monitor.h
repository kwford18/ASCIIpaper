#pragma once

#include <memory>

namespace ASCIIpaper::Engine {

    class SystemMonitor {
    public:
        SystemMonitor();

        ~SystemMonitor(); 

        // Called every frame, but only polls the OS internally once per second
        void Update(float deltaTime); 

        float GetCpuUsage() const { return m_cpuUsage; }
        float GetRamUsage() const { return m_ramUsage; }

    private:
        float m_cpuUsage = 0.0f;
        float m_ramUsage = 0.0f;
        float m_pollTimer = 0.0f;

        struct PlatformData;
        std::unique_ptr<PlatformData> m_platformData;
    };

} // namespace ASCIIpaper::Engine