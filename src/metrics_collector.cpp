#include "metrics_collector.h"
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>

SystemMetrics MetricsCollector::collectSystemMetrics() {
    SystemMetrics metrics{};

    // Lightweight simulated values to keep the demo portable.
    // In a real embedded target, these would be read from /proc, netlink, or vendor SDKs.
    metrics.cpuUsage = 23.5;
    metrics.memoryUsage = 41.2;
    metrics.interfaceName = "eth0";

    return metrics;
}
