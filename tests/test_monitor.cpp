#include <gtest/gtest.h>
#include "engine/system_monitor.h"

using namespace ASCIIpaper::Engine;

TEST(SystemMonitorTest, RespectsPollTimer) {
    SystemMonitor monitor;
    
    float initialCpu = monitor.GetCpuUsage();
    
    // Update by only half a second (should NOT trigger a new OS read)
    monitor.Update(0.5f);
    EXPECT_EQ(monitor.GetCpuUsage(), initialCpu);
    
    // Update past the 1.0s threshold (SHOULD trigger an OS read)
    monitor.Update(0.6f);
    // Now usage might have changed based on hardware
}