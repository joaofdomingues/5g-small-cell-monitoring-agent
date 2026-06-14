#include "agent_loop.h"
#include "logger.h"

#include <chrono>
#include <csignal>
#include <iostream>

// ---------------------------------------------------------------------------
// Signal handler — async-signal-safe (only touches an atomic)
// ---------------------------------------------------------------------------
namespace {
void handleSignal(int) {
    g_shutdown.store(true, std::memory_order_relaxed);
}
} // namespace

// ---------------------------------------------------------------------------
// AgentLoop
// ---------------------------------------------------------------------------

AgentLoop::AgentLoop(const AgentConfig& config,
                     MetricsCollector&  collector,
                     NetworkMonitor&    monitor,
                     AlertCallback      onAlert)
    : config_(config)
    , collector_(collector)
    , monitor_(monitor)
    , onAlert_(std::move(onAlert))
{
    std::signal(SIGTERM, handleSignal);
    std::signal(SIGINT,  handleSignal);
}

AgentLoop::~AgentLoop() {
    // Ensure threads see the shutdown flag and wake from any wait.
    stop();
    slotCv_.notify_all();
    if (collectorThr_.joinable()) collectorThr_.join();
    if (evaluatorThr_.joinable()) evaluatorThr_.join();
}

/*static*/ void AgentLoop::stop() {
    g_shutdown.store(true, std::memory_order_relaxed);
}

// ---------------------------------------------------------------------------
// One-shot (CI / --once mode)
// ---------------------------------------------------------------------------
int AgentLoop::runOnce() {
    const SystemMetrics  sys     = collector_.collectSystemMetrics();
    const TelecomMetrics telecom = config_.metrics;
    const auto           alerts  = monitor_.evaluateMetrics(telecom, config_.thresholds);

    monitor_.printAlerts(alerts);
    if (onAlert_ && !alerts.empty()) onAlert_(alerts);

    return static_cast<int>(alerts.size());
}

// ---------------------------------------------------------------------------
// Collector thread — produces CollectorResult every pollIntervalSec
// ---------------------------------------------------------------------------
void AgentLoop::collectorThread() {
    Logger::info("[collector] thread started");

    while (!g_shutdown.load(std::memory_order_relaxed)) {
        CollectorResult result;
        result.system  = collector_.collectSystemMetrics();
        result.telecom = config_.metrics;

        {
            std::lock_guard<std::mutex> lock(slotMutex_);
            slot_      = result;
            slotReady_ = true;
        }
        slotCv_.notify_one();   // wake the evaluator

        // Sleep in 100 ms slices so SIGTERM is noticed promptly.
        const int slices = config_.pollIntervalSec * 10;
        for (int i = 0; i < slices; ++i) {
            if (g_shutdown.load(std::memory_order_relaxed)) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    // Wake evaluator so it can exit its wait.
    slotCv_.notify_all();
    Logger::info("[collector] thread exiting");
}

// ---------------------------------------------------------------------------
// Evaluator thread — consumes CollectorResult and evaluates alerts
// ---------------------------------------------------------------------------
void AgentLoop::evaluatorThread() {
    Logger::info("[evaluator] thread started");
    int cycle = 0;

    while (true) {
        CollectorResult result;

        {
            std::unique_lock<std::mutex> lock(slotMutex_);
            // Wait until the collector posts a result or shutdown is requested.
            slotCv_.wait(lock, [this] {
                return slotReady_ ||
                       g_shutdown.load(std::memory_order_relaxed);
            });

            if (g_shutdown.load(std::memory_order_relaxed) && !slotReady_) break;
            result     = *slot_;
            slotReady_ = false;
        }

        ++cycle;
        Logger::info("--- Poll cycle #" + std::to_string(cycle) + " ---");

        std::cout
            << "\n[5G SMALL CELL AGENT]\n"
            << "Cell ID:           " << result.telecom.cellId       << "\n"
            << "Technology:        " << result.telecom.technology   << "\n"
            << "CPU Usage:         " << result.system.cpuUsage      << "%\n"
            << "Memory Usage:      " << result.system.memoryUsage   << "%\n"
            << "Network Interface: " << result.system.interfaceName << "\n"
            << "SINR:              " << result.telecom.sinr         << " dB\n"
            << "RSRP:              " << result.telecom.rsrp         << " dBm\n"
            << "Latency:           " << result.telecom.latencyMs    << " ms\n"
            << "Packet Loss:       " << result.telecom.packetLoss   << "%\n"
            << "Cell Status:       " << result.telecom.status       << "\n\n";

        const auto alerts = monitor_.evaluateMetrics(
            result.telecom, config_.thresholds);
        monitor_.printAlerts(alerts);

        if (onAlert_ && !alerts.empty()) onAlert_(alerts);
    }

    Logger::info("[evaluator] thread exiting after " +
                 std::to_string(cycle) + " cycle(s).");
}

// ---------------------------------------------------------------------------
// run() — launches both threads and blocks until g_shutdown
// ---------------------------------------------------------------------------
void AgentLoop::run() {
    Logger::info("Agent entering continuous mode (interval=" +
                 std::to_string(config_.pollIntervalSec) +
                 "s). Send SIGTERM or SIGINT to stop.");

    collectorThr_ = std::thread(&AgentLoop::collectorThread, this);
    evaluatorThr_ = std::thread(&AgentLoop::evaluatorThread, this);

    // Block main thread until shutdown — threads are joined in destructor.
    collectorThr_.join();
    evaluatorThr_.join();
}
