# Upgrade Notes

## v2 — Improvements for production alignment

### C++ — Real system metrics from /proc

`metrics_collector.cpp` now reads live data from the Linux kernel:

- **CPU usage** — two `/proc/stat` snapshots 100 ms apart (delta method).
- **Memory usage** — `MemTotal` and `MemAvailable` from `/proc/meminfo`.
- **Network interface** — first non-loopback entry from `/proc/net/dev`.

`#ifdef _WIN32` guards keep the code portable; on Windows the fields fall
back to safe defaults (extend with `GetSystemTimes` / `GlobalMemoryStatusEx`
if needed).

### C++ — SSH diagnostics with real command execution

`ssh_diagnostics.cpp` now uses `popen`/`pclose` to execute diagnostic
commands:

- **Local mode** — when host is `localhost` or `127.0.0.1`, commands run
  directly on the current machine.  Safe for CI and demos.
- **Remote mode** — a non-interactive `ssh` invocation with `ConnectTimeout`,
  `BatchMode=yes` and `StrictHostKeyChecking=no` (appropriate for a lab
  environment; use a CA cert in production).

### C++ — GoogleTest integration

`CMakeLists.txt` now fetches GoogleTest v1.14.0 via `FetchContent` and
builds `test_network_monitor_gtest`, which covers:

- Healthy metrics produce no alerts
- Degraded metrics produce six alerts with correct codes
- `CELL_NOT_ACTIVE` severity is `CRITICAL`
- `LOW_SINR` severity is `WARNING`
- Boundary condition: SINR exactly at threshold does not trigger
- Boundary condition: SINR one below threshold triggers
- `SSH_DISABLED` alert when `sshEnabled = false`

The original assert-based `test_network_monitor` remains available as a
zero-dependency fallback.

### Python — Paramiko SSH health check

`scripts/ssh_health_check.py` now uses `paramiko` for remote SSH:

- Supports key-based auth (`--key`), password auth (`--password`) and
  agent-forwarded keys.
- `--dry-run` or `--host localhost` runs commands locally.
- Structured `DiagResult` dataclass with per-command success tracking.
- Exit code 1 if any check fails.

### CI — SonarCloud step

`.github/workflows/ci.yml` now includes a SonarCloud scan step.  It is
gated on `SONAR_TOKEN` being set so the pipeline passes in forks without
the secret.  `CMAKE_EXPORT_COMPILE_COMMANDS=ON` is set so SonarQube can
perform C++ analysis.
