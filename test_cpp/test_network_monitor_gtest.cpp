#include <gtest/gtest.h>
#include "network_monitor.h"
#include <algorithm>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static TelecomMetrics healthyMetrics() {
    return {"AVEIRO-SC-001", "5G", -86, -11, 22, 18, 0.3, "ACTIVE", true};
}

static TelecomMetrics degradedMetrics() {
    return {"AVEIRO-SC-002", "5G", -112, -18, 7, 92, 2.4, "DEGRADED", false};
}

static Thresholds defaultThresholds() {
    return {};  // Uses struct defaults: minSinr=10, minRsrp=-100, etc.
}

// ---------------------------------------------------------------------------
// Alert evaluation tests
// ---------------------------------------------------------------------------

TEST(NetworkMonitorTest, HealthyMetricsProduceNoAlerts) {
    NetworkMonitor monitor;
    const auto alerts = monitor.evaluateMetrics(healthyMetrics(), defaultThresholds());
    EXPECT_TRUE(alerts.empty());
}

TEST(NetworkMonitorTest, DegradedMetricsProduceSixAlerts) {
    NetworkMonitor monitor;
    const auto alerts = monitor.evaluateMetrics(degradedMetrics(), defaultThresholds());
    EXPECT_EQ(alerts.size(), 6u);
}

TEST(NetworkMonitorTest, DegradedAlertCodesAreCorrect) {
    NetworkMonitor monitor;
    const auto alerts = monitor.evaluateMetrics(degradedMetrics(), defaultThresholds());
    ASSERT_GE(alerts.size(), 6u);
    EXPECT_EQ(alerts[0].code, "LOW_SINR");
    EXPECT_EQ(alerts[1].code, "WEAK_RSRP");
    EXPECT_EQ(alerts[2].code, "HIGH_PACKET_LOSS");
    EXPECT_EQ(alerts[3].code, "HIGH_LATENCY");
    EXPECT_EQ(alerts[4].code, "CELL_NOT_ACTIVE");
    EXPECT_EQ(alerts[5].code, "SSH_DISABLED");
}

TEST(NetworkMonitorTest, CellNotActiveAlertIsCritical) {
    NetworkMonitor monitor;
    const auto alerts = monitor.evaluateMetrics(degradedMetrics(), defaultThresholds());
    const auto it = std::find_if(alerts.begin(), alerts.end(),
        [](const Alert& a){ return a.code == "CELL_NOT_ACTIVE"; });
    ASSERT_NE(it, alerts.end());
    EXPECT_EQ(it->severity, "CRITICAL");
}

TEST(NetworkMonitorTest, LowSinrAlertIsWarning) {
    NetworkMonitor monitor;
    const auto alerts = monitor.evaluateMetrics(degradedMetrics(), defaultThresholds());
    const auto it = std::find_if(alerts.begin(), alerts.end(),
        [](const Alert& a){ return a.code == "LOW_SINR"; });
    ASSERT_NE(it, alerts.end());
    EXPECT_EQ(it->severity, "WARNING");
}

TEST(NetworkMonitorTest, BoundaryThresholdSinrExactlyAtLimitProducesNoAlert) {
    NetworkMonitor monitor;
    Thresholds t = defaultThresholds();
    t.minSinr = 10;
    TelecomMetrics m = healthyMetrics();
    m.sinr = 10; // exactly at limit — should NOT trigger
    const auto alerts = monitor.evaluateMetrics(m, t);
    const bool hasSinrAlert = std::any_of(alerts.begin(), alerts.end(),
        [](const Alert& a){ return a.code == "LOW_SINR"; });
    EXPECT_FALSE(hasSinrAlert);
}

TEST(NetworkMonitorTest, BoundaryThresholdSinrOneBelowLimitProducesAlert) {
    NetworkMonitor monitor;
    Thresholds t = defaultThresholds();
    t.minSinr = 10;
    TelecomMetrics m = healthyMetrics();
    m.sinr = 9; // one below limit — should trigger
    const auto alerts = monitor.evaluateMetrics(m, t);
    const bool hasSinrAlert = std::any_of(alerts.begin(), alerts.end(),
        [](const Alert& a){ return a.code == "LOW_SINR"; });
    EXPECT_TRUE(hasSinrAlert);
}

TEST(NetworkMonitorTest, SshDisabledAlertWhenSshEnabledFalse) {
    NetworkMonitor monitor;
    TelecomMetrics m = healthyMetrics();
    m.sshEnabled = false;
    const auto alerts = monitor.evaluateMetrics(m, defaultThresholds());
    const bool hasSshAlert = std::any_of(alerts.begin(), alerts.end(),
        [](const Alert& a){ return a.code == "SSH_DISABLED"; });
    EXPECT_TRUE(hasSshAlert);
}
