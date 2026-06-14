#include "network_monitor.h"
#include "logger.h"

#include <fstream>
#include <regex>
#include <sstream>
#include <stdexcept>

namespace {
std::string readFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open config file: " + filepath);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string extractString(const std::string& json, const std::string& key, const std::string& fallback) {
    std::regex pattern("\\\"" + key + "\\\"\\s*:\\s*\\\"([^\\\"]*)\\\"");
    std::smatch match;
    if (std::regex_search(json, match, pattern)) {
        return match[1];
    }
    return fallback;
}

int extractInt(const std::string& json, const std::string& key, int fallback) {
    std::regex pattern("\\\"" + key + "\\\"\\s*:\\s*(-?[0-9]+)");
    std::smatch match;
    if (std::regex_search(json, match, pattern)) {
        return std::stoi(match[1]);
    }
    return fallback;
}

double extractDouble(const std::string& json, const std::string& key, double fallback) {
    std::regex pattern("\\\"" + key + "\\\"\\s*:\\s*(-?[0-9]+(\\.[0-9]+)?)");
    std::smatch match;
    if (std::regex_search(json, match, pattern)) {
        return std::stod(match[1]);
    }
    return fallback;
}

bool extractBool(const std::string& json, const std::string& key, bool fallback) {
    std::regex pattern("\\\"" + key + "\\\"\\s*:\\s*(true|false)");
    std::smatch match;
    if (std::regex_search(json, match, pattern)) {
        return match[1] == "true";
    }
    return fallback;
}
}

AgentConfig NetworkMonitor::readAgentConfig(const std::string& filepath) const {
    const std::string json = readFile(filepath);
    AgentConfig config{};

    config.metrics.cellId = extractString(json, "cell_id", "AVEIRO-SC-001");
    config.metrics.technology = extractString(json, "technology", "5G");
    config.metrics.rsrp = extractInt(json, "rsrp", -86);
    config.metrics.rsrq = extractInt(json, "rsrq", -11);
    config.metrics.sinr = extractInt(json, "sinr", 22);
    config.metrics.latencyMs = extractInt(json, "latency_ms", 18);
    config.metrics.packetLoss = extractDouble(json, "packet_loss", 0.3);
    config.metrics.status = extractString(json, "status", "ACTIVE");
    config.metrics.sshEnabled = extractBool(json, "ssh_enabled", true);

    config.thresholds.minSinr = extractInt(json, "min_sinr", 10);
    config.thresholds.minRsrp = extractInt(json, "min_rsrp", -100);
    config.thresholds.maxPacketLoss = extractDouble(json, "max_packet_loss", 1.0);
    config.thresholds.maxLatencyMs = extractInt(json, "max_latency_ms", 50);
    config.logFile = extractString(json, "log_file", "logs/agent.log");

    config.pollIntervalSec = extractInt(json, "poll_interval_sec", 5);
    config.daemonMode = extractBool(json, "daemon_mode", false);

    return config;
}

TelecomMetrics NetworkMonitor::readTelecomMetrics(const std::string& filepath) const {
    return readAgentConfig(filepath).metrics;
}

std::vector<Alert> NetworkMonitor::evaluateMetrics(const TelecomMetrics& metrics, const Thresholds& thresholds) const {
    std::vector<Alert> alerts;

    if (metrics.sinr < thresholds.minSinr) {
        alerts.push_back({"LOW_SINR", "WARNING", "SINR below threshold. Radio quality may be degraded."});
    }
    if (metrics.rsrp < thresholds.minRsrp) {
        alerts.push_back({"WEAK_RSRP", "WARNING", "RSRP below threshold. Coverage may be poor."});
    }
    if (metrics.packetLoss > thresholds.maxPacketLoss) {
        alerts.push_back({"HIGH_PACKET_LOSS", "WARNING", "Packet loss above threshold."});
    }
    if (metrics.latencyMs > thresholds.maxLatencyMs) {
        alerts.push_back({"HIGH_LATENCY", "WARNING", "Latency above threshold."});
    }
    if (metrics.status != "ACTIVE") {
        alerts.push_back({"CELL_NOT_ACTIVE", "CRITICAL", "Cell operational status is not ACTIVE."});
    }
    if (!metrics.sshEnabled) {
        alerts.push_back({"SSH_DISABLED", "WARNING", "SSH diagnostics are disabled for this node."});
    }

    return alerts;
}

void NetworkMonitor::printAlerts(const std::vector<Alert>& alerts) const {
    if (alerts.empty()) {
        Logger::info("No telecom health alerts detected");
        return;
    }

    Logger::warn("Telecom health alerts detected: " + std::to_string(alerts.size()));
    for (const auto& alert : alerts) {
        const std::string line = alert.code + " [" + alert.severity + "] - " + alert.message;
        if (alert.severity == "CRITICAL") {
            Logger::error(line);
        } else {
            Logger::warn(line);
        }
    }
}
