cmake_minimum_required(VERSION 3.14)
project(5g_small_cell_monitoring_agent VERSION 3.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(TARGET_ARCH "Target architecture: native | aarch64" "native")
option(EMBEDDED_HW "Enable real hardware register access" OFF)

if(EMBEDDED_HW)
    add_compile_definitions(EMBEDDED_HW)
    message(STATUS "EMBEDDED_HW register access enabled")
else()
    message(STATUS "HAL software stub (development host)")
endif()

include_directories(include src)
enable_testing()

find_package(Threads REQUIRED)

# ── Core library ─────────────────────────────────────────────────────────────
add_library(5g_agent_core
    src/agent_loop.cpp
    src/logger.cpp
    src/metrics_collector.cpp
    src/network_monitor.cpp
    src/ssh_diagnostics.cpp
)
target_link_libraries(5g_agent_core Threads::Threads)

# ── Main executable ───────────────────────────────────────────────────────────
add_executable(5g_agent src/main.cpp)
target_link_libraries(5g_agent 5g_agent_core)

# ── CTest targets ─────────────────────────────────────────────────────────────
add_executable(test_network_monitor test_cpp/test_network_monitor.cpp)
target_link_libraries(test_network_monitor 5g_agent_core)
add_test(NAME network_monitor_ctest COMMAND test_network_monitor)

add_executable(test_agent_loop test_cpp/test_agent_loop.cpp)
target_link_libraries(test_agent_loop 5g_agent_core)
add_test(NAME agent_loop_ctest COMMAND test_agent_loop)

# ── GoogleTest (optional) ─────────────────────────────────────────────────────
option(ENABLE_GTEST "Build GoogleTest tests" ON)
if(ENABLE_GTEST)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
        GIT_SHALLOW    TRUE
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_executable(test_network_monitor_gtest
        test_cpp/test_network_monitor_gtest.cpp
    )
    target_link_libraries(test_network_monitor_gtest 5g_agent_core GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(test_network_monitor_gtest)
endif()
