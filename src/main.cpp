#include "agent_loop.h"
#include "logger.h"
#include "metrics_collector.h"
#include "network_monitor.h"
#include "ssh_diagnostics.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

// ---------------------------------------------------------------------------
// CLI
// ---------------------------------------------------------------------------
namespace {

struct CliArgs {
    std::string configPath{"config/agent_config.json"};
    bool        runOnce{false};
    bool        help{false};
};

CliArgs parseArgs(int argc, char* argv[]) {
    CliArgs args;
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if ((a == "--config" || a == "-c") && i + 1 < argc)
            args.configPath = argv[++i];
        else if (a == "--once")
            args.runOnce = true;
        else if (a == "--help" || a == "-h")
            args.help = true;
        else if (a[0] != '-')
            args.configPath = a;
    }
    return args;
}

void printHelp() {
    std::cout
        << "Usage: ./5g_agent [--config <path>] [--once] [--help]\n\n"
        << "  --config <path>  JSON config  (default: config/agent_config.json)\n"
        << "  --once           Single poll cycle then exit  (CI / test mode)\n\n"
        << "Threading (continuous mode):\n"
        << "  collector thread  → reads /proc + config every pollIntervalSec\n"
        << "  evaluator thread  → wakes on condition_variable, evaluates alerts\n"
        << "  main thread       → blocks until SIGTERM / SIGINT\n\n"
        << "Exit codes:  0=healthy  1=error  2=alerts detected\n";
}

} // namespace

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    try {
        const CliArgs args = parseArgs(argc, argv);
        if (args.help) { printHelp(); return 0; }

        // ── Bootstrap ──────────────────────────────────────────────────────
        NetworkMonitor monitor;
        AgentConfig    config = monitor.readAgentConfig(args.configPath);

        Logger::configureFile(config.logFile);
        Logger::info("Starting 5G Small Cell Monitoring Agent");
        Logger::info("Config: " + args.configPath);

        // std::unique_ptr — explicit RAII ownership
        auto collector = std::make_unique<MetricsCollector>();
        auto ssh       = std::make_unique<SSHDiagnostics>(config.metrics.sshEnabled);

        if (ssh->isEnabled()) {
            ssh->runDiagnostics("localhost", "user");
        }

        // ── Alert callback (extensible: could write to syslog, SNMP, etc.) ─
        AlertCallback onAlert = [](const std::vector<Alert>& alerts) {
            Logger::warn("Active alerts: " + std::to_string(alerts.size()));
        };

        // ── Run ───────────────────────────────────────────────────────────
        AgentLoop loop(config, *collector, monitor, onAlert);

        if (args.runOnce || !config.daemonMode) {
            // One-shot: single poll, exit (CI / --once / default)
            const int alertCount = loop.runOnce();
            Logger::info("Agent execution completed");
            return alertCount > 0 ? 2 : 0;
        } else {
            // Continuous: collector thread + evaluator thread until SIGTERM
            loop.run();
            Logger::info("Agent shutdown complete");
            return 0;
        }

    } catch (const std::exception& ex) {
        Logger::error(std::string("Fatal error: ") + ex.what());
        return 1;
    }
}
