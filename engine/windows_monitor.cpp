#ifdef WIN32
#include "engine/system_monitor.h"
#include <windows.h>

namespace ASCIIpaper::Engine {

// Define the hidden platform data specifically for Windows
struct SystemMonitor::PlatformData {
    unsigned long long prevIdle = 0;
    unsigned long long prevKernel = 0;
    unsigned long long prevUser = 0;
};

// Helper to convert FILETIME to uint64_t
static unsigned long long FileTimeToInt64(const FILETIME& ft) {
    return (static_cast<unsigned long long>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
}

// Initialize the unique pointer in the constructor
SystemMonitor::SystemMonitor() : m_platformData(std::make_unique<PlatformData>()) {
    // Preliminary read so first CPU calculation is not massive spike
    FILETIME idleTime, kernelTime, userTime;
    if (GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        m_platformData->prevIdle = FileTimeToInt64(idleTime);
        m_platformData->prevKernel = FileTimeToInt64(kernelTime);
        m_platformData->prevUser = FileTimeToInt64(userTime);
    }
}

SystemMonitor::~SystemMonitor() = default;

// Polling Logic
void SystemMonitor::Update(float deltaTime) {
    m_pollTimer += deltaTime;
    if (m_pollTimer < 1.0f)
        return;
    m_pollTimer = 0.0f;

    // Get RAM Usage
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        m_ramUsage = static_cast<float>(memInfo.dwMemoryLoad);
    }

    // Get CPU Usage
    FILETIME idleTime, kernelTime, userTime;
    if (GetSystemTimes(&idleTime, &kernelTime, &userTime)) {
        unsigned long long idle = FileTimeToInt64(idleTime);
        unsigned long long kernel = FileTimeToInt64(kernelTime);
        unsigned long long user = FileTimeToInt64(userTime);

        unsigned long long diffIdle = idle - m_platformData->prevIdle;
        unsigned long long diffKernel = kernel - m_platformData->prevKernel;
        unsigned long long diffUser = user - m_platformData->prevUser;
        unsigned long long total = diffKernel + diffUser;

        if (total > 0 && m_platformData->prevIdle != 0) {
            m_cpuUsage = static_cast<float>(total - diffIdle) / static_cast<float>(total) * 100.0f;
        }

        m_platformData->prevIdle = idle;
        m_platformData->prevKernel = kernel;
        m_platformData->prevUser = user;
    }
}

} // namespace ASCIIpaper::Engine
#endif