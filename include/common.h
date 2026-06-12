#pragma once

#include <string>
#include <vector>

struct SystemMetrics {
    double cpuUsage{};
    double memoryUsage{};
    std::string interfaceName;
};

struct TelecomMetrics {
    std::string cellId;
    std::string technology;
    int rsrp{};
    int rsrq{};
    int sinr{};
    int latencyMs{};
    double packetLoss{};
    std::string status;
    bool sshEnabled{};
};

struct Thresholds {
    int minSinr{10};
    int minRsrp{-100};
    double maxPacketLoss{1.0};
    int maxLatencyMs{50};
};

struct AgentConfig {
    TelecomMetrics metrics;
    Thresholds thresholds;
    std::string logFile{"logs/agent.log"};
};

struct Alert {
    std::string code;
    std::string severity;
    std::string message;
};

