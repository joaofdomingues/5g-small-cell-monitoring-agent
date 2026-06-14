# Demo — 5G Small Cell Monitoring Agent

## What this project demonstrates

This project simulates a Linux-based C++ embedded monitoring agent for a 5G small cell environment.

It demonstrates:

- C++17 software development
- Linux/CMake build process
- Python simulation and testing scripts
- Telecom metrics: RSRP, RSRQ, SINR, latency and packet loss
- SSH diagnostics script
- GitHub Actions CI
- SonarQube configuration
- Optional Docker/Kubernetes files

## Tested commands

```bash
mkdir build
cd build
cmake ..
cmake --build .
./5g_agent
```

## Runtime output

```txt
[INFO] Starting 5G Small Cell Monitoring Agent

[5G SMALL CELL AGENT]
Cell ID: AVEIRO-SC-001
Technology: 5G
CPU Usage: 23.5%
Memory Usage: 41.2%
Network Interface: eth0
Latency: 18 ms
Packet Loss: 0.3%
RSRP: -86 dBm
RSRQ: -11 dB
SINR: 22 dB
Cell Status: ACTIVE
SSH Diagnostics: ENABLED

[INFO] Agent execution completed
```

## Tests

```txt
pytest -q tests
....                                                                     [100%]
4 passed in 0.06s
```
