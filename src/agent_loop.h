#pragma once

#include "common.h"
#include "metrics_collector.h"
#include "network_monitor.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <optional>
#include <thread>

// ---------------------------------------------------------------------------
// CollectorResult — data produced by the collector thread each cycle
// ---------------------------------------------------------------------------
struct CollectorResult {
    SystemMetrics  system;
    TelecomMetrics telecom;
};

// ---------------------------------------------------------------------------
// AgentLoop — production-grade monitoring loop
//
// Threading model
// ───────────────
//   collector thread  → reads /proc and telecom config every pollIntervalSec,
//                       writes CollectorResult into shared slot
//   evaluator thread  → wakes on condition_variable, evaluates alerts,
//                       calls the user-supplied onAlert callback
//   main thread       → calls run(), blocks until g_shutdown
//
// Synchronisation
// ───────────────
//   std::mutex + std::condition_variable for collector→evaluator handoff.
//   std::atomic<bool> g_shutdown for SIGTERM/SIGINT (async-signal-safe).
//
// RAII
// ────
//   Destructor sets g_shutdown and joins both worker threads.
//   Non-copyable, non-movable.
// ---------------------------------------------------------------------------

using AlertCallback = std::function<void(const std::vector<Alert>&)>;

class AgentLoop {
public:
    explicit AgentLoop(const AgentConfig&   config,
                       MetricsCollector&    collector,
                       NetworkMonitor&      monitor,
                       AlertCallback        onAlert = nullptr);
    ~AgentLoop();

    AgentLoop(const AgentLoop&)            = delete;
    AgentLoop& operator=(const AgentLoop&) = delete;
    AgentLoop(AgentLoop&&)                 = delete;
    AgentLoop& operator=(AgentLoop&&)      = delete;

    // One-shot: collect + evaluate once, return alert count. (CI / --once)
    int runOnce();

    // Continuous: run collector + evaluator threads until g_shutdown.
    void run();

    // Async-signal-safe shutdown — safe to call from SIGTERM handler.
    static void stop();

private:
    // ── Collector thread ───────────────────────────────────────────────────
    void collectorThread();

    // ── Evaluator thread ──────────────────────────────────────────────────
    void evaluatorThread();

    // ── Shared state (collector → evaluator) ──────────────────────────────
    std::mutex                   slotMutex_;
    std::condition_variable      slotCv_;
    std::optional<CollectorResult> slot_;   // set by collector, consumed by evaluator
    bool                         slotReady_{false};

    // ── Owned threads ─────────────────────────────────────────────────────
    std::thread collectorThr_;
    std::thread evaluatorThr_;

    // ── Dependencies (non-owning references) ──────────────────────────────
    const AgentConfig& config_;
    MetricsCollector&  collector_;
    NetworkMonitor&    monitor_;
    AlertCallback      onAlert_;
};
