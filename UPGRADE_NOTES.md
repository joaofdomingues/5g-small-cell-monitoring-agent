# Upgrade Notes

This upgraded version was created to make the project stronger for an Embedded Software Developer / 5G Small Cell application.

## What was improved

- Added `--config` CLI support.
- Added healthy and degraded configuration scenarios.
- Added threshold-based alert detection.
- Added explicit alert codes:
  - `LOW_SINR`
  - `WEAK_RSRP`
  - `HIGH_PACKET_LOSS`
  - `HIGH_LATENCY`
  - `CELL_NOT_ACTIVE`
  - `SSH_DISABLED`
- Added log file generation under `logs/agent.log`.
- Added C++ alert evaluation tests through CTest.
- Improved Python tests from 4 to 6 tests.
- Improved simulation script with `--mode healthy` and `--mode degraded`.
- Improved README with job-requirement mapping, limitations and next steps.
- Improved `.gitignore` to avoid committing build artifacts, virtual environments and logs.
- Improved GitHub Actions workflow to build C++, run C++ tests, run healthy/degraded scenarios and run Python tests.

## Validated locally

```txt
C++ build: OK
C++ tests with CTest: OK
Healthy scenario execution: OK, exit code 0
Degraded scenario execution: OK, exit code 2
Python tests: 6 passed
```

## Recommended GitHub repository name

```txt
5g-small-cell-monitoring-agent
```
