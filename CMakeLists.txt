cmake_minimum_required(VERSION 3.10)
project(5g_small_cell_monitoring_agent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include src)

enable_testing()

add_library(5g_agent_core
    src/metrics_collector.cpp
    src/network_monitor.cpp
    src/ssh_diagnostics.cpp
    src/logger.cpp
)

add_executable(5g_agent
    src/main.cpp
)
target_link_libraries(5g_agent 5g_agent_core)

add_executable(test_network_monitor
    test_cpp/test_network_monitor.cpp
)
target_link_libraries(test_network_monitor 5g_agent_core)

add_test(NAME network_monitor_alert_tests COMMAND test_network_monitor)
