#pragma once

#include <atomic>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// System metrics (populated from /proc on Linux, fallback on Windows)
// ---------------------------------------------------------------------------
struct SystemMetrics {
    double      cpuUsage{};
    double      memoryUsage{};
    std::string interfaceName;
};

// ---------------------------------------------------------------------------
// Telecom / radio metrics for a single 5G small cell
// ---------------------------------------------------------------------------
struct TelecomMetrics {
    std::string cellId;
    std::string technology;
    int         rsrp{};          // Reference Signal Received Power  (dBm)
    int         rsrq{};          // Reference Signal Received Quality (dB)
    int         sinr{};          // Signal-to-Interference-plus-Noise Ratio (dB)
    int         latencyMs{};
    double      packetLoss{};
    std::string status;
    bool        sshEnabled{};
};

// ---------------------------------------------------------------------------
// Alert evaluation thresholds (configurable via JSON)
// ---------------------------------------------------------------------------
struct Thresholds {
    int    minSinr{10};
    int    minRsrp{-100};
    double maxPacketLoss{1.0};
    int    maxLatencyMs{50};
};

// ---------------------------------------------------------------------------
// Full agent configuration (parsed from agent_config.json)
// ---------------------------------------------------------------------------
struct AgentConfig {
    TelecomMetrics metrics;
    Thresholds     thresholds;
    std::string    logFile{"logs/agent.log"};
    int            pollIntervalSec{5};   // Continuous-mode poll interval
    bool           daemonMode{false};    // Run as background daemon
};

// ---------------------------------------------------------------------------
// Alert produced by the evaluation engine
// ---------------------------------------------------------------------------
struct Alert {
    std::string code;
    std::string severity;   // "WARNING" | "CRITICAL"
    std::string message;
};

// ---------------------------------------------------------------------------
// Global shutdown flag — set by SIGTERM / SIGINT handler
// ---------------------------------------------------------------------------
inline std::atomic<bool> g_shutdown{false};
