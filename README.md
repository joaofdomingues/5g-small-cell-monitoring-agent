# 5G Small Cell Monitoring Agent

A C++ Linux-based monitoring agent that simulates an embedded-style software component for a 5G small cell environment.

This project was created to demonstrate practical knowledge of C++, Linux, Python, telecom-oriented metrics, SSH diagnostics, software testing, CI/CD and integration workflows in the context of embedded telecom systems.

> Honest positioning: this is a simulation/portfolio project. It is not production firmware and it does not claim real small-cell hardware integration.

---

## Project Overview

The project simulates a monitoring agent running inside or alongside a 5G small cell system.

The agent loads a JSON configuration file, reads telecom-oriented metrics, evaluates thresholds, generates alerts and writes operational logs.

It covers metrics such as:

- CPU usage
- Memory usage
- Network interface status
- Latency
- Packet loss
- RSRP
- RSRQ
- SINR
- Cell operational status
- SSH diagnostics status

---

## Why This Project Exists

This project was built to align with Embedded Software Developer roles focused on 5G small cell solutions.

It demonstrates practical exposure to:

- C++ software development for Linux platforms
- Embedded-style monitoring logic
- Telecom network metrics
- Python-based simulation and testing
- SSH diagnostic workflows
- CI/CD with GitHub Actions
- Static analysis readiness with SonarQube
- Optional container/orchestration awareness through Docker and Kubernetes files

---

## Architecture

```txt
5g-small-cell-monitoring-agent/
│
├── src/                  C++ monitoring agent source code
├── include/              Shared C++ data structures
├── test_cpp/             C++ alert evaluation tests
├── scripts/              Python simulation and SSH diagnostic scripts
├── tests/                Python automated tests
├── config/               Healthy/degraded agent configuration files
├── docker/               Dockerfile for optional container usage
├── k8s/                  Kubernetes manifests
├── .github/workflows/    GitHub Actions CI pipeline
├── CMakeLists.txt        C++ build and test configuration
├── sonar-project.properties
├── PROFESSIONAL_DEMO.html
└── README.md
```

---

## Main Features

- C++ monitoring agent designed for Linux environments
- JSON-based configuration input
- Healthy and degraded 5G small cell scenarios
- Telecom metrics including RSRP, RSRQ and SINR
- Network health indicators such as latency and packet loss
- Threshold-based alert detection
- Log file generation under `logs/agent.log`
- Python scripts for metric simulation and diagnostics
- SSH diagnostic simulation
- Automated Python tests with pytest
- C++ alert evaluation tests with CTest
- CMake-based C++ build
- GitHub Actions CI configuration
- SonarQube configuration file
- Optional Docker and Kubernetes files
- Professional HTML demo page

---

## Build Instructions

### Requirements

Install the required system dependencies:

```bash
sudo apt update
sudo apt install -y cmake build-essential python3 python3-pip python3-venv
```

### Build the C++ agent and C++ tests

```bash
rm -rf build
mkdir build
cd build
cmake ..
cmake --build .
cd ..
```

The main executable will be generated as:

```bash
build/5g_agent
```

---

## Run the C++ Agent

### Healthy scenario

```bash
./build/5g_agent --config config/agent_config.json
```

Example output:

```txt
[INFO] Starting 5G Small Cell Monitoring Agent
[INFO] Using config file: config/agent_config.json

[5G SMALL CELL AGENT]
Cell ID: AVEIRO-SC-001
Technology: 5G
CPU Usage: 23.5%
Memory Usage: 41.2%
Network Interface: eth0
Latency: 28 ms
Packet Loss: 0.35%
RSRP: -96 dBm
RSRQ: -12 dB
SINR: 18 dB
Cell Status: ACTIVE
SSH Diagnostics: ENABLED

[INFO] No telecom health alerts detected
[INFO] Agent execution completed
```

### Degraded scenario

```bash
./build/5g_agent --config config/degraded_config.json
```

Example output:

```txt
[WARNING] Telecom health alerts detected: 6
[WARNING] LOW_SINR - SINR below threshold. Radio quality may be degraded.
[WARNING] WEAK_RSRP - RSRP below threshold. Coverage may be poor.
[WARNING] HIGH_PACKET_LOSS - Packet loss above threshold.
[WARNING] HIGH_LATENCY - Latency above threshold.
[ERROR] CELL_NOT_ACTIVE - Cell operational status is not ACTIVE.
[WARNING] SSH_DISABLED [WARNING] - SSH diagnostics are disabled for this node.
```

The degraded scenario intentionally exits with code `2` to indicate that alerts were detected.

---

## Python Environment

Create and activate a virtual environment:

```bash
python3 -m venv .venv
source .venv/bin/activate
```

Install test dependencies:

```bash
python -m pip install --upgrade pip
python -m pip install -r requirements.txt
```

---

## Run Python Tests

```bash
python -m pytest tests/
```

Expected result:

```txt
6 passed
```

---

## Run C++ Tests

```bash
cd build
ctest --output-on-failure
cd ..
```

Expected result:

```txt
100% tests passed
```

---

## Run the 5G Metrics Simulation Script

Generate a healthy scenario:

```bash
python scripts/simulate_5g_metrics.py --mode healthy --output config/agent_config.json
```

Generate a degraded scenario:

```bash
python scripts/simulate_5g_metrics.py --mode degraded --output config/degraded_config.json
```

Example output:

```json
{
  "cell_id": "AVEIRO-SC-001",
  "technology": "5G",
  "ssh_enabled": true,
  "min_sinr": 10,
  "min_rsrp": -100,
  "max_packet_loss": 1.0,
  "max_latency_ms": 50,
  "log_file": "logs/agent.log",
  "rsrp": -96,
  "rsrq": -12,
  "sinr": 18,
  "latency_ms": 28,
  "packet_loss": 0.35,
  "status": "ACTIVE"
}
```

---

## Mapping to Job Requirements

| Job Requirement | Project Implementation |
|---|---|
| C/C++ | Core monitoring agent written in C++ |
| Python | Metric simulation, diagnostics and automated tests |
| Linux platforms | Built and tested in Linux/WSL |
| Windows environments | Can be executed from Windows through WSL |
| Embedded software solutions | Simulates an embedded-style monitoring agent for a 5G small cell environment |
| Firmware components | Represents monitoring logic that could run alongside device firmware |
| Telecommunications networks | Uses telecom-oriented metrics such as RSRP, RSRQ, SINR, latency and packet loss |
| SSH protocol | Includes SSH diagnostics simulation |
| Software testing | Includes Python tests with pytest and C++ tests with CTest |
| Git | Version-controlled and published on GitHub |
| GitHub Projects | Suitable for issue tracking and task management |
| SonarQube | Includes SonarQube configuration |
| Kubernetes | Includes optional Kubernetes manifests |
| VMware | Can be tested in a Linux VM environment |
| 4G/5G knowledge | Includes healthy and degraded 5G small cell scenarios |
## What This Project Demonstrates

This project demonstrates the ability to:

- understand technical requirements from an embedded/telecom job description;
- build a working C++ Linux-based tool around those requirements;
- simulate telecom network metrics;
- detect network/radio degradation through threshold-based alerts;
- validate behaviour with Python and C++ automated tests;
- document technical decisions clearly;
- prepare a project for GitHub and technical review.

---

## Limitations

This is a simulation project.

It is not production firmware and does not currently include:

- real embedded hardware execution;
- ARM cross-compilation;
- real radio hardware integration;
- real SSH connections to telecom devices;
- low-level drivers;
- RTOS integration;
- Yocto or Buildroot setup;
- live 5G network stack integration.

These limitations are intentional and clearly documented to keep the project honest and technically defensible.

---

## Possible Next Steps

Future improvements could include:

- ARM cross-compilation support;
- GoogleTest or Catch2 integration instead of lightweight assert-based C++ tests;
- real SSH diagnostics against a test VM;
- Docker-based reproducible environment;
- GitHub Actions status badge in the README after publishing;
- basic local dashboard for metric visualization;
- integration with real system metrics from `/proc` or netlink.

---

## Professional Positioning

This project is suitable as a portfolio project for junior/intermediate roles involving:

- Embedded Software Development
- C++ development for Linux
- Telecom software
- 4G/5G infrastructure
- Network monitoring
- Integration and testing workflows
