#pragma once

#include "common.h"
#include <string>
#include <vector>

class NetworkMonitor {
public:
    AgentConfig readAgentConfig(const std::string& filepath) const;
    TelecomMetrics readTelecomMetrics(const std::string& filepath) const;
    std::vector<Alert> evaluateMetrics(const TelecomMetrics& metrics, const Thresholds& thresholds) const;
    void printAlerts(const std::vector<Alert>& alerts) const;
};
