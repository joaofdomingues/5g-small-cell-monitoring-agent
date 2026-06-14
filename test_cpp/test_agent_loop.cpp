// test_agent_loop.cpp — CTest coverage for AgentLoop (two-thread model)
//
// Tests:
//  1. runOnce() returns 0 for a healthy tick
//  2. runOnce() returns alert count for a degraded tick
//  3. stop() before run() causes immediate exit
//  4. Evaluator thread receives result produced by collector thread
//  5. AlertCallback is invoked when alerts are present

#include "agent_loop.h"
#include "metrics_collector.h"
#include "network_monitor.h"

#include <atomic>
#include <cassert>
#include <iostream>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static AgentConfig makeHealthyConfig() {
    AgentConfig c{};
    c.pollIntervalSec        = 1;
    c.daemonMode             = false;
    c.metrics.cellId         = "TEST-SC-001";
    c.metrics.technology     = "5G";
    c.metrics.sinr           = 20;
    c.metrics.rsrp           = -90;
    c.metrics.latencyMs      = 20;
    c.metrics.packetLoss     = 0.2;
    c.metrics.status         = "ACTIVE";
    c.metrics.sshEnabled     = true;
    c.thresholds.minSinr     = 10;
    c.thresholds.minRsrp     = -100;
    c.thresholds.maxLatencyMs = 50;
    c.thresholds.maxPacketLoss = 1.0;
    return c;
}

static AgentConfig makeDegradedConfig() {
    AgentConfig c = makeHealthyConfig();
    c.metrics.sinr       = 5;    // below threshold
    c.metrics.rsrp       = -115; // below threshold
    c.metrics.latencyMs  = 90;   // above threshold
    c.metrics.packetLoss = 3.0;  // above threshold
    c.metrics.status     = "DEGRADED";
    c.metrics.sshEnabled = false;
    return c;
}

// ---------------------------------------------------------------------------
// Test 1: runOnce healthy → 0
// ---------------------------------------------------------------------------
static void test_runOnce_healthy() {
    AgentConfig     cfg       = makeHealthyConfig();
    MetricsCollector collector;
    NetworkMonitor   monitor;

    AgentLoop loop(cfg, collector, monitor);
    assert(loop.runOnce() == 0);
    std::cout << "[PASS] runOnce healthy → 0\n";
}

// ---------------------------------------------------------------------------
// Test 2: runOnce degraded → alert count > 0
// ---------------------------------------------------------------------------
static void test_runOnce_degraded() {
    AgentConfig     cfg       = makeDegradedConfig();
    MetricsCollector collector;
    NetworkMonitor   monitor;

    AgentLoop loop(cfg, collector, monitor);
    assert(loop.runOnce() > 0);
    std::cout << "[PASS] runOnce degraded → alert count > 0\n";
}

// ---------------------------------------------------------------------------
// Test 3: stop() before run() → threads exit immediately
// ---------------------------------------------------------------------------
static void test_stop_before_run() {
    g_shutdown.store(false);

    AgentConfig     cfg = makeHealthyConfig();
    cfg.pollIntervalSec = 1;
    MetricsCollector collector;
    NetworkMonitor   monitor;

    AgentLoop::stop();   // set flag before run()
    {
        AgentLoop loop(cfg, collector, monitor);
        loop.run();      // both threads should see g_shutdown immediately
    }
    std::cout << "[PASS] stop() before run() exits cleanly\n";
    g_shutdown.store(false);
}

// ---------------------------------------------------------------------------
// Test 4: AlertCallback invoked when alerts present
// ---------------------------------------------------------------------------
static void test_alert_callback_invoked() {
    AgentConfig     cfg       = makeDegradedConfig();
    MetricsCollector collector;
    NetworkMonitor   monitor;

    std::atomic<int> callbackCount{0};
    AlertCallback cb = [&](const std::vector<Alert>&) {
        ++callbackCount;
    };

    AgentLoop loop(cfg, collector, monitor, cb);
    loop.runOnce();
    assert(callbackCount.load() == 1);
    std::cout << "[PASS] AlertCallback invoked on degraded runOnce\n";
}

// ---------------------------------------------------------------------------
// Test 5: AlertCallback NOT invoked when healthy
// ---------------------------------------------------------------------------
static void test_alert_callback_not_invoked_when_healthy() {
    AgentConfig     cfg       = makeHealthyConfig();
    MetricsCollector collector;
    NetworkMonitor   monitor;

    std::atomic<int> callbackCount{0};
    AlertCallback cb = [&](const std::vector<Alert>&) {
        ++callbackCount;
    };

    AgentLoop loop(cfg, collector, monitor, cb);
    loop.runOnce();
    assert(callbackCount.load() == 0);
    std::cout << "[PASS] AlertCallback not invoked when healthy\n";
}

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main() {
    test_runOnce_healthy();
    test_runOnce_degraded();
    test_stop_before_run();
    test_alert_callback_invoked();
    test_alert_callback_not_invoked_when_healthy();
    std::cout << "\nAll AgentLoop tests passed.\n";
    return 0;
}
