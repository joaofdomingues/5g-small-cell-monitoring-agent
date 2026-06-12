#include "logger.h"
#include "metrics_collector.h"
#include "network_monitor.h"
#include "ssh_diagnostics.h"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

struct RuntimeOptions {
    std::string configPath = "config/agent_config.json";
    std::string scenario = "healthy";
};

RuntimeOptions parseRuntimeOptions(int argc, char* argv[]) {
    RuntimeOptions options;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if ((arg == "--config" || arg == "-c") && i + 1 < argc) {
            options.configPath = argv[++i];
        } else if ((arg == "--scenario" || arg == "-s") && i + 1 < argc) {
            options.scenario = argv[++i];
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage:\n";
            std::cout << "  ./5g_agent\n";
            std::cout << "  ./5g_agent config/agent_config.json\n";
            std::cout << "  ./5g_agent config/agent_config.json healthy\n";
            std::cout << "  ./5g_agent config/agent_config.json degraded\n";
            std::cout << "  ./5g_agent --config config/agent_config.json --scenario healthy\n";
            std::cout << "  ./5g_agent --config config/agent_config.json --scenario degraded\n";
            std::exit(0);
        } else if (arg == "healthy" || arg == "degraded") {
            options.scenario = arg;
        } else {
            options.configPath = arg;
        }
    }

    if (options.scenario != "healthy" && options.scenario != "degraded") {
        std::cerr << "[ERROR] Invalid scenario: " << options.scenario << "\n";
        std::cerr << "Valid scenarios: healthy, degraded\n";
        std::exit(1);
    }

    return options;
}

void applyDegradedScenario(TelecomMetrics& telecom) {
    telecom.sinr = 5;
    telecom.rsrp = -118;
    telecom.rsrq = -19;
    telecom.latencyMs = 180;
    telecom.packetLoss = 6.5;
    telecom.status = "DEGRADED";
}

} // namespace

int main(int argc, char* argv[]) {
    try {
        const RuntimeOptions options = parseRuntimeOptions(argc, argv);
        const std::string configPath = options.configPath;
        const std::string scenario = options.scenario;

        MetricsCollector collector;
        NetworkMonitor networkMonitor;

        AgentConfig config = networkMonitor.readAgentConfig(configPath);
        Logger::configureFile(config.logFile);

        Logger::info("Starting 5G Small Cell Monitoring Agent");
        Logger::info("Using config file: " + configPath);
        Logger::info("Using scenario: " + scenario);

        SSHDiagnostics sshDiagnostics(config.metrics.sshEnabled);
        SystemMetrics system = collector.collectSystemMetrics();
        TelecomMetrics telecom = config.metrics;

        if (scenario == "degraded") {
            applyDegradedScenario(telecom);
        }

        std::cout << "\n[5G SMALL CELL AGENT]\n";
        std::cout << "Cell ID: " << telecom.cellId << "\n";
        std::cout << "Technology: " << telecom.technology << "\n";
        std::cout << "CPU Usage: " << system.cpuUsage << "%\n";
        std::cout << "Memory Usage: " << system.memoryUsage << "%\n";
        std::cout << "Network Interface: " << system.interfaceName << "\n";
        std::cout << "Latency: " << telecom.latencyMs << " ms\n";
        std::cout << "Packet Loss: " << telecom.packetLoss << "%\n";
        std::cout << "RSRP: " << telecom.rsrp << " dBm\n";
        std::cout << "RSRQ: " << telecom.rsrq << " dB\n";
        std::cout << "SINR: " << telecom.sinr << " dB\n";
        std::cout << "Cell Status: " << telecom.status << "\n";
        std::cout << "SSH Diagnostics: " << (sshDiagnostics.isEnabled() ? "ENABLED" : "DISABLED") << "\n\n";

        const auto alerts = networkMonitor.evaluateMetrics(telecom, config.thresholds);
        networkMonitor.printAlerts(alerts);

        Logger::info("Agent execution completed");

        return alerts.empty() ? 0 : 2;
    } catch (const std::exception& ex) {
        Logger::error(ex.what());
        return 1;
    }
}