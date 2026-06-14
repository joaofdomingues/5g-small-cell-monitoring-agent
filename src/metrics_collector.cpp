#include "metrics_collector.h"

#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <chrono>

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

namespace {

// Read two consecutive /proc/stat snapshots 100 ms apart and compute CPU %.
// Falls back to 0.0 if /proc/stat is unavailable (non-Linux or permission issue).
double readCpuUsage() {
#ifdef _WIN32
    return 0.0; // /proc not available on Windows; extend with GetSystemTimes() if needed.
#else
    auto readStat = [](long long& idle, long long& total) -> bool {
        std::ifstream stat("/proc/stat");
        if (!stat.is_open()) return false;
        std::string cpu;
        long long user, nice, system, idle_, iowait, irq, softirq, steal;
        stat >> cpu >> user >> nice >> system >> idle_ >> iowait >> irq >> softirq >> steal;
        idle  = idle_ + iowait;
        total = user + nice + system + idle_ + iowait + irq + softirq + steal;
        return true;
    };

    long long idle1 = 0, total1 = 0, idle2 = 0, total2 = 0;
    if (!readStat(idle1, total1)) return 0.0;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    if (!readStat(idle2, total2)) return 0.0;

    const long long totalDiff = total2 - total1;
    const long long idleDiff  = idle2  - idle1;
    if (totalDiff <= 0) return 0.0;

    return 100.0 * (1.0 - static_cast<double>(idleDiff) / static_cast<double>(totalDiff));
#endif
}

// Parse /proc/meminfo for MemTotal and MemAvailable and return usage %.
// Falls back to 0.0 if /proc/meminfo is unavailable.
double readMemoryUsage() {
#ifdef _WIN32
    return 0.0; // Extend with GlobalMemoryStatusEx() if needed.
#else
    std::ifstream meminfo("/proc/meminfo");
    if (!meminfo.is_open()) return 0.0;

    long long memTotal = 0, memAvailable = 0;
    std::string line;
    while (std::getline(meminfo, line)) {
        std::istringstream iss(line);
        std::string key;
        long long value;
        iss >> key >> value;
        if (key == "MemTotal:")     memTotal     = value;
        if (key == "MemAvailable:") memAvailable = value;
        if (memTotal && memAvailable) break;
    }

    if (memTotal <= 0) return 0.0;
    return 100.0 * (1.0 - static_cast<double>(memAvailable) / static_cast<double>(memTotal));
#endif
}

// Return the name of the first non-loopback network interface found in
// /proc/net/dev, or "eth0" as a safe fallback.
std::string readPrimaryInterface() {
#ifdef _WIN32
    return "Ethernet"; // Fallback for Windows environments.
#else
    std::ifstream netdev("/proc/net/dev");
    if (!netdev.is_open()) return "eth0";

    std::string line;
    std::getline(netdev, line); // header line 1
    std::getline(netdev, line); // header line 2
    while (std::getline(netdev, line)) {
        std::istringstream iss(line);
        std::string iface;
        iss >> iface;
        // Strip trailing colon
        if (!iface.empty() && iface.back() == ':') iface.pop_back();
        if (iface != "lo") return iface;
    }
    return "eth0";
#endif
}

} // namespace

// ---------------------------------------------------------------------------
// MetricsCollector
// ---------------------------------------------------------------------------

SystemMetrics MetricsCollector::collectSystemMetrics() {
    SystemMetrics metrics{};
    metrics.cpuUsage      = readCpuUsage();
    metrics.memoryUsage   = readMemoryUsage();
    metrics.interfaceName = readPrimaryInterface();
    return metrics;
}
