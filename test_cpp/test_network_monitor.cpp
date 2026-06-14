#include "network_monitor.h"

#include <cassert>
#include <iostream>

void testHealthyMetricsProduceNoAlerts() {
    NetworkMonitor monitor;
    TelecomMetrics metrics{"AVEIRO-SC-001", "5G", -86, -11, 22, 18, 0.3, "ACTIVE", true};
    Thresholds thresholds{};
    const auto alerts = monitor.evaluateMetrics(metrics, thresholds);
    assert(alerts.empty());
}

void testDegradedMetricsProduceAlerts() {
    NetworkMonitor monitor;
    TelecomMetrics metrics{"AVEIRO-SC-002", "5G", -112, -18, 7, 92, 2.4, "DEGRADED", false};
    Thresholds thresholds{};
    const auto alerts = monitor.evaluateMetrics(metrics, thresholds);
    assert(alerts.size() == 6);
    assert(alerts[0].code == "LOW_SINR");
    assert(alerts[1].code == "WEAK_RSRP");
    assert(alerts[2].code == "HIGH_PACKET_LOSS");
    assert(alerts[3].code == "HIGH_LATENCY");
    assert(alerts[4].code == "CELL_NOT_ACTIVE");
    assert(alerts[5].code == "SSH_DISABLED");
}

int main() {
    testHealthyMetricsProduceNoAlerts();
    testDegradedMetricsProduceAlerts();
    std::cout << "C++ alert evaluation tests passed\n";
    return 0;
}
