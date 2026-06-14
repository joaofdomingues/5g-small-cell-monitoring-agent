# 5G Small Cell Monitoring Agent

> **Portfolio project** — simulated 5G small cell monitoring agent for Linux.  
> All metrics and radio parameters are configurable via JSON. No real radio hardware required.

---

## What this demonstrates

| Skill | Implementation |
|---|---|
| C++17 | 7 modules, RAII, `std::unique_ptr`, `std::atomic`, signal handling |
| Linux embedded | `/proc/stat`, `/proc/meminfo`, `/proc/net/dev` live reads |
| Firmware / HAL | `firmware_stub.h` — register map, GPIO, `#ifdef EMBEDDED_HW` |
| Cross-compilation | CMake `TARGET_ARCH=aarch64` + ARM64 toolchain support |
| SSH protocol | `ssh_diagnostics.cpp` via `popen/pclose`; Python via Paramiko |
| Telecom networks | RSRP, RSRQ, SINR, latency, packet loss, cell status (4G/5G) |
| Continuous operation | `AgentLoop` — poll loop with SIGTERM/SIGINT graceful shutdown |
| Testing | GoogleTest (7 tests) + CTest (4 tests) + pytest (15 tests) |
| CI/CD | GitHub Actions: build → CTest → GoogleTest → run → pytest → SonarCloud |
| Kubernetes | `k8s/deployment.yaml` + `k8s/service.yaml` |

---

## Build & run

```bash
# Build (native)
mkdir build && cd build
cmake .. -DENABLE_GTEST=ON
cmake --build .

# One-shot mode (CI / test)
./5g_agent --once --config config/agent_config.json

# Continuous mode (daemon_mode: true in JSON)
./5g_agent --config config/agent_config.json
# Send SIGTERM or Ctrl+C to stop gracefully

# Cross-compile for ARM64
cmake .. -DTARGET_ARCH=aarch64 \
         -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++

# Enable real hardware register access
cmake .. -DEMBEDDED_HW=ON
```

## Test

```bash
cd build && ctest --output-on-failure   # CTest (NetworkMonitor + AgentLoop)
python -m pytest tests/ -v             # Python (simulate + SSH)
```

## Generate metrics

```bash
python scripts/simulate_5g_metrics.py --mode healthy   # → config/agent_config.json
python scripts/simulate_5g_metrics.py --mode degraded  # → config/agent_config.json
python scripts/ssh_health_check.py --dry-run           # Local SSH diagnostics
```

## Architecture

```
src/
  main.cpp              — CLI, RAII ownership, tick lambda
  agent_loop.cpp/h      — Continuous poll loop, SIGTERM handler
  metrics_collector.cpp — /proc/stat, /proc/meminfo, /proc/net/dev
  network_monitor.cpp   — JSON config parse, alert evaluation
  ssh_diagnostics.cpp   — popen/pclose SSH execution
  logger.cpp            — File + stdout logging
  firmware_stub.h       — HAL register map, GPIO, ARM64 cross-compile
scripts/
  simulate_5g_metrics.py — Config generator (healthy/degraded/random)
  ssh_health_check.py    — Paramiko SSH diagnostics (dry-run + remote)
tests/
  test_network_parser.py — 15 pytest tests for config generation
test_cpp/
  test_network_monitor.cpp      — CTest: alert evaluation
  test_agent_loop.cpp           — CTest: loop + SIGTERM
  test_network_monitor_gtest.cpp — GoogleTest: 7 tests with boundary conditions
```

## What I would add with real hardware

- Replace `firmware_stub.h` fallbacks with actual memory-mapped register reads (UIO driver or `/dev/mem`)
- Replace `popen` SSH with `libssh2` for proper async non-blocking SSH
- Add Yocto recipe for packaging the agent as a system service
- Integrate with `systemd` watchdog (`sd_notify(WATCHDOG=1)`) for production reliability
- Add Prometheus metrics endpoint for observability
