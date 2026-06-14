#!/usr/bin/env bash
set -e
mkdir -p build
cd build
cmake ..
make
cd ..
./build/5g_agent config/agent_config.json
